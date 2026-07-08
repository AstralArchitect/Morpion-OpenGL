#pragma once

#include <queue>
#include <mutex>
#include <variant>
#include <vector>
#include <glm/glm.hpp>

// Define the types of events that the logic can send to the renderer

struct EventPlacePiece {
    int type; // 1 for cross, 2 for circle
    glm::mat4 target_matrix;
    float start_time;
};

struct EventAnimatePiece {
    int render_index;
    glm::mat4 current_matrix;
};

struct EventGameWon {
    std::vector<int> win_indexes;
};

struct EventResetGame {};

// Variant holding any of the possible renderer events
using RendererEvent = std::variant<EventPlacePiece, EventAnimatePiece, EventGameWon, EventResetGame>;

// A thread-safe communication bus between game logic and the renderer
class LogicRendererCommunication {
public:
    void pushEvent(const RendererEvent& event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_eventQueue.push(event);
    }

    bool pollEvent(RendererEvent& out_event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_eventQueue.empty()) return false;
        out_event = m_eventQueue.front();
        m_eventQueue.pop();
        return true;
    }

private:
    std::queue<RendererEvent> m_eventQueue;
    std::mutex m_mutex;
};

// Global instance for communication to be used across the application
extern LogicRendererCommunication g_messageBus;
