#pragma once

class Event {
public:
    virtual ~Event() = default;

    bool is_cancelled() const { return m_cancelled; }
    void cancel() { m_cancelled = true; }

private:
    bool m_cancelled = false;
};