#include "render/dx12/dx12_hook.h"

#include "modules/module_manager.h"
#include "ui/menu.h"
#include "ui/ui_settings.h"
#include "utils/logger.h"

#include <Windows.h>
#include <atomic>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include <kiero.h>
#include <mutex>
#include <vector>
#include <input/input_manager.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
    constexpr UINT kFramesInFlight = 3;

    using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain3*, UINT, UINT);
    using ResizeBuffersFn = HRESULT(__stdcall*)(IDXGISwapChain3*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
    using ExecuteCommandListsFn = void(__stdcall*)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

    PresentFn g_originalPresent = nullptr;
    ResizeBuffersFn g_originalResizeBuffers = nullptr;
    ExecuteCommandListsFn g_originalExecuteCommandLists = nullptr;

    HWND g_window = nullptr;
    WNDPROC g_originalWndProc = nullptr;

    ID3D12Device* g_device = nullptr;
    ID3D12CommandQueue* g_commandQueue = nullptr;
    ID3D12DescriptorHeap* g_rtvHeap = nullptr;
    ID3D12DescriptorHeap* g_srvHeap = nullptr;
    ID3D12GraphicsCommandList* g_commandList = nullptr;
    ID3D12Fence* g_fence = nullptr;
    HANDLE g_fenceEvent = nullptr;

    struct FrameContext {
        ID3D12CommandAllocator* allocator = nullptr;
        ID3D12Resource* renderTarget = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
        UINT64 fenceValue = 0;
    };

    std::vector<FrameContext> g_frames;
    UINT g_bufferCount = 0;
    UINT g_rtvDescriptorSize = 0;
    DXGI_FORMAT g_renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    UINT64 g_nextFenceValue = 1;

    std::mutex g_mutex;
    std::atomic_bool g_initialized = false;
    std::atomic_bool g_shuttingDown = false;
    std::atomic_bool g_menuOpen = false;
    std::atomic_int g_skipFrames = 0;
    std::atomic<unsigned long long> g_resumeRenderAtTick = 0;

    void request_render_pause(unsigned int frames, unsigned int milliseconds);

    int mouse_message_key(UINT msg, WPARAM wparam, bool& down) {
        switch (msg) {
        case WM_LBUTTONDOWN:
            down = true;
            return VK_LBUTTON;
        case WM_LBUTTONUP:
            down = false;
            return VK_LBUTTON;
        case WM_RBUTTONDOWN:
            down = true;
            return VK_RBUTTON;
        case WM_RBUTTONUP:
            down = false;
            return VK_RBUTTON;
        case WM_MBUTTONDOWN:
            down = true;
            return VK_MBUTTON;
        case WM_MBUTTONUP:
            down = false;
            return VK_MBUTTON;
        case WM_XBUTTONDOWN:
            down = true;
            return HIWORD(wparam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
        case WM_XBUTTONUP:
            down = false;
            return HIWORD(wparam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
        default:
            return 0;
        }
    }

    LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        const bool keyDown = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        const bool keyUp = (msg == WM_KEYUP || msg == WM_SYSKEYUP);

        if ((keyDown && wparam == VK_F11) || msg == WM_SIZE || msg == WM_DISPLAYCHANGE) {
            request_render_pause(180, 3000);
        }

        if (keyUp && wparam == VK_INSERT) {
            const bool opening = !g_menuOpen.load();
            g_menuOpen.store(opening);
            return 0;
        }

        const bool consumedByInput = InputManager::instance().handle_win32_message(msg, wparam, lparam);
        if (consumedByInput) {
            return 0;
        }

        if (g_menuOpen.load() && ImGui::GetCurrentContext()) {
            ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);

            ImGuiIO& io = ImGui::GetIO();
            const bool isMouseMsg = (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST);

            if ((isMouseMsg && io.WantCaptureMouse) || (!isMouseMsg && io.WantCaptureKeyboard)) {
                return TRUE;
            }
        }

        return CallWindowProc(g_originalWndProc, hwnd, msg, wparam, lparam);
    }

    void request_render_pause(unsigned int frames, unsigned int milliseconds) {
        g_skipFrames.store(static_cast<int>(frames));
        g_resumeRenderAtTick.store(GetTickCount64() + milliseconds);
    }

    void wait_for_fence(UINT64 fenceValue) {
        if (!g_fence || !g_fenceEvent || fenceValue == 0) {
            return;
        }

        if (g_fence->GetCompletedValue() >= fenceValue) {
            return;
        }

        if (SUCCEEDED(g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent))) {
            WaitForSingleObject(g_fenceEvent, INFINITE);
        }
    }

    void wait_for_overlay_work() {
        for (FrameContext& frame : g_frames) {
            wait_for_fence(frame.fenceValue);
            frame.fenceValue = 0;
        }
    }

    void apply_overlay_style() {
        ui::apply_theme();
    }

    void release_frame_resources() {
        wait_for_overlay_work();

        for (FrameContext& frame : g_frames) {
            if (frame.renderTarget) {
                frame.renderTarget->Release();
                frame.renderTarget = nullptr;
            }

            if (frame.allocator) {
                frame.allocator->Release();
                frame.allocator = nullptr;
            }
        }

        g_frames.clear();
        g_bufferCount = 0;

        if (g_rtvHeap) {
            g_rtvHeap->Release();
            g_rtvHeap = nullptr;
        }
    }

    void cleanup_imgui() {
        if (!g_initialized.exchange(false)) {
            return;
        }

        LOG_INFO("Cleaning up ImGui & DX12 render resources...");

        wait_for_overlay_work();

        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (g_originalWndProc && g_window) {
            SetWindowLongPtr(g_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_originalWndProc));
            g_originalWndProc = nullptr;
        }

        release_frame_resources();

        if (g_commandList) {
            g_commandList->Release();
            g_commandList = nullptr;
        }

        if (g_srvHeap) {
            g_srvHeap->Release();
            g_srvHeap = nullptr;
        }

        if (g_fence) {
            g_fence->Release();
            g_fence = nullptr;
        }

        if (g_fenceEvent) {
            CloseHandle(g_fenceEvent);
            g_fenceEvent = nullptr;
        }

        if (g_device) {
            g_device->Release();
            g_device = nullptr;
        }

        g_window = nullptr;
        g_commandQueue = nullptr;
        LOG_INFO("ImGui & DX12 render resources cleaned up successfully.");
    }

    bool create_render_targets(IDXGISwapChain3* swapChain) {
        DXGI_SWAP_CHAIN_DESC desc{};
        if (FAILED(swapChain->GetDesc(&desc))) {
            return false;
        }

        g_bufferCount = desc.BufferCount;
        g_frames.resize(g_bufferCount);

        D3D12_DESCRIPTOR_HEAP_DESC rtvDesc{};
        rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvDesc.NumDescriptors = g_bufferCount;
        rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        if (FAILED(g_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_rtvHeap)))) {
            return false;
        }

        g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < g_bufferCount; ++i) {
            FrameContext& frame = g_frames[i];

            if (FAILED(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.allocator)))) {
                return false;
            }

            if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&frame.renderTarget)))) {
                return false;
            }

            frame.rtvHandle = rtvHandle;
            g_device->CreateRenderTargetView(frame.renderTarget, nullptr, frame.rtvHandle);
            rtvHandle.ptr += g_rtvDescriptorSize;
        }

        return true;
    }

    bool initialize_imgui(IDXGISwapChain3* swapChain) {
        if (!g_commandQueue) {
            return false;
        }

        DXGI_SWAP_CHAIN_DESC desc{};
        if (FAILED(swapChain->GetDesc(&desc)) || !desc.OutputWindow) {
            return false;
        }

        g_window = desc.OutputWindow;
        g_renderTargetFormat = desc.BufferDesc.Format == DXGI_FORMAT_UNKNOWN
            ? DXGI_FORMAT_R8G8B8A8_UNORM
            : desc.BufferDesc.Format;

        LOG_INFO("Initializing ImGui DX12 overlay for window handle: {:p}...", static_cast<void*>(g_window));

        if (FAILED(swapChain->GetDevice(IID_PPV_ARGS(&g_device)))) {
            LOG_ERROR("Failed to acquire D3D12 device from swapchain!");
            return false;
        }

        D3D12_DESCRIPTOR_HEAP_DESC srvDesc{};
        srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvDesc.NumDescriptors = 1;
        srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        if (FAILED(g_device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_srvHeap)))) {
            LOG_ERROR("Failed to create SRV descriptor heap!");
            return false;
        }

        if (!create_render_targets(swapChain)) {
            LOG_ERROR("Failed to create render target views!");
            return false;
        }

        if (FAILED(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)))) {
            LOG_ERROR("Failed to create D3D12 synchronization fence!");
            return false;
        }

        g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!g_fenceEvent) {
            LOG_ERROR("Failed to create fence event handle!");
            return false;
        }

        if (FAILED(g_device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            g_frames[0].allocator,
            nullptr,
            IID_PPV_ARGS(&g_commandList)))) {
            LOG_ERROR("Failed to create graphics command list!");
            return false;
        }
        g_commandList->Close();

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        static const char* fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
        if (std::filesystem::exists(fontPath)) {
            io.Fonts->AddFontFromFileTTF(fontPath, 22.0f);
        }

        apply_overlay_style();
        ui_settings::reset_ui_scale_cache();
        ImGui_ImplWin32_Init(g_window);
        ImGui_ImplDX12_Init(
            g_device,
            static_cast<int>(kFramesInFlight),
            g_renderTargetFormat,
            g_srvHeap,
            g_srvHeap->GetCPUDescriptorHandleForHeapStart(),
            g_srvHeap->GetGPUDescriptorHandleForHeapStart());

        g_originalWndProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(g_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wnd_proc)));

        g_initialized.store(true);
        LOG_INFO("ImGui DX12 overlay initialized successfully.");
        return true;
    }

    void render_imgui(IDXGISwapChain3* swapChain) {
        const UINT index = swapChain->GetCurrentBackBufferIndex();
        if (index >= g_frames.size()) {
            return;
        }

        FrameContext& frame = g_frames[index];
        wait_for_fence(frame.fenceValue);
        frame.fenceValue = 0;

        frame.allocator->Reset();

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = frame.renderTarget;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        g_commandList->Reset(frame.allocator, nullptr);
        g_commandList->ResourceBarrier(1, &barrier);
        g_commandList->OMSetRenderTargets(1, &frame.rtvHandle, FALSE, nullptr);
        g_commandList->SetDescriptorHeaps(1, &g_srvHeap);

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ui_settings::apply_ui_scale();
        ImGui::NewFrame();

        ModuleManager::instance().on_imgui_render();

        if (g_menuOpen.load()) {
            menu::render();
        }

        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_commandList);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_commandList->ResourceBarrier(1, &barrier);
        g_commandList->Close();

        ID3D12CommandList* lists[] = { g_commandList };
        g_commandQueue->ExecuteCommandLists(1, lists);

        const UINT64 fenceValue = g_nextFenceValue++;
        if (SUCCEEDED(g_commandQueue->Signal(g_fence, fenceValue))) {
            frame.fenceValue = fenceValue;
        }
    }

    void __stdcall hook_execute_command_lists(ID3D12CommandQueue* queue, UINT count, ID3D12CommandList* const* lists) {
        if (!g_commandQueue && queue) {
            g_commandQueue = queue;
        }

        g_originalExecuteCommandLists(queue, count, lists);
    }

    HRESULT __stdcall hook_present(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags) {
        if (!g_shuttingDown.load()) {
            std::lock_guard lock(g_mutex);

            const int skipFrames = g_skipFrames.load();
            const unsigned long long now = GetTickCount64();
            const bool inCooldown = now < g_resumeRenderAtTick.load();

            if (skipFrames > 0 || inCooldown) {
                if (skipFrames > 0) {
                    g_skipFrames.store(skipFrames - 1);
                }

                if (g_initialized.load()) {
                    cleanup_imgui();
                }

                return g_originalPresent(swapChain, syncInterval, flags);
            }

            if (!g_initialized.load()) {
                initialize_imgui(swapChain);
            }

            if (g_initialized.load()) {
                render_imgui(swapChain);
            }
        }

        return g_originalPresent(swapChain, syncInterval, flags);
    }

    HRESULT __stdcall hook_resize_buffers(
        IDXGISwapChain3* swapChain,
        UINT bufferCount,
        UINT width,
        UINT height,
        DXGI_FORMAT newFormat,
        UINT swapChainFlags) {
        std::lock_guard lock(g_mutex);

        if (g_initialized.load()) {
            cleanup_imgui();
        }

        return g_originalResizeBuffers(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
    }
}

namespace dx12_hook {
    bool start(HMODULE) {
        g_shuttingDown.store(false);

        LOG_INFO("Initializing Kiero D3D12 hook...");

        if (kiero::init(kiero::RenderType::D3D12) != kiero::Status::Success) {
            LOG_ERROR("Failed to initialize Kiero D3D12!");
            return false;
        }

        constexpr uint16_t executeCommandListsIndex = 54;
        constexpr uint16_t presentIndex = 140;
        constexpr uint16_t resizeBuffersIndex = 145;

        if (kiero::bind(executeCommandListsIndex, reinterpret_cast<void**>(&g_originalExecuteCommandLists), hook_execute_command_lists) != kiero::Status::Success) {
            LOG_ERROR("Failed to bind ExecuteCommandLists hook!");
            return false;
        }

        if (kiero::bind(presentIndex, reinterpret_cast<void**>(&g_originalPresent), hook_present) != kiero::Status::Success) {
            LOG_ERROR("Failed to bind Present hook!");
            return false;
        }

        if (kiero::bind(resizeBuffersIndex, reinterpret_cast<void**>(&g_originalResizeBuffers), hook_resize_buffers) != kiero::Status::Success) {
            LOG_ERROR("Failed to bind ResizeBuffers hook!");
            return false;
        }

        LOG_INFO("DX12 hooks bound successfully.");
        return true;
    }

    void stop() {
        LOG_INFO("Stopping DX12 hook...");
        g_shuttingDown.store(true);

        {
            std::lock_guard lock(g_mutex);
            cleanup_imgui();
        }

        kiero::shutdown();
        LOG_INFO("DX12 hook stopped.");
    }

    bool is_menu_open() {
        return g_menuOpen.load();
    }

    void set_menu_open(bool open) {
        g_menuOpen.store(open);
    }
}