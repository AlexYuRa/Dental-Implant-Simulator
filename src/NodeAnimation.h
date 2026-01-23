#pragma once
/**
 * NodeAnimation.h - Simple Node-Level Transform Animation
 * 
 * For GLB models exported from Blender with object-level keyframes (no armature).
 * Supports position, rotation, and scale interpolation between two keyframes.
 * 
 * Usage:
 *   NodeAnimation anim;
 *   anim.loadFromModel(model);           // Extract keyframes from GLB
 *   anim.trigger();                      // Start animation on click
 *   anim.update(deltaTime);              // Call each frame
 *   glm::mat4 transform = anim.getTransform();  // Apply to model matrix
 */

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/scene.h>
#include <iostream>

class Model;  // Forward declaration

// ============================================================================
// NodeAnimation - Simple transform animation between two keyframes
// ============================================================================
class NodeAnimation {
public:
    // Animation state
    enum class State {
        AT_START,       // Resting at initial position
        ANIMATING_TO_END,   // Moving to final position
        AT_END,         // Resting at final position
        ANIMATING_TO_START  // Returning to initial position
    };

private:
    // Keyframe data (position only for simplicity - extend if needed)
    glm::vec3 m_startPosition = glm::vec3(0.0f);
    glm::vec3 m_endPosition = glm::vec3(0.0f);
    
    glm::quat m_startRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::quat m_endRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    
    glm::vec3 m_startScale = glm::vec3(1.0f);
    glm::vec3 m_endScale = glm::vec3(1.0f);

    // Current interpolated values
    glm::vec3 m_currentPosition = glm::vec3(0.0f);
    glm::quat m_currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 m_currentScale = glm::vec3(1.0f);

    // Animation parameters
    float m_duration = 0.5f;          // Seconds to complete animation
    float m_currentTime = 0.0f;       // Current progress
    State m_state = State::AT_START;
    bool m_hasAnimation = false;

public:
    NodeAnimation() = default;

    // Set animation duration in seconds
    void setDuration(float seconds) { m_duration = seconds; }
    float getDuration() const { return m_duration; }

    // Check if animation data was loaded successfully
    bool hasAnimation() const { return m_hasAnimation; }

    // Get current state
    State getState() const { return m_state; }
    bool isAnimating() const { 
        return m_state == State::ANIMATING_TO_END || m_state == State::ANIMATING_TO_START; 
    }

    // Manually set keyframes (alternative to loading from model)
    void setKeyframes(const glm::vec3& startPos, const glm::vec3& endPos) {
        m_startPosition = startPos;
        m_endPosition = endPos;
        m_currentPosition = startPos;
        m_hasAnimation = true;
        m_state = State::AT_START;
    }

    // Set keyframes with full transform
    void setKeyframes(
        const glm::vec3& startPos, const glm::quat& startRot, const glm::vec3& startScale,
        const glm::vec3& endPos, const glm::quat& endRot, const glm::vec3& endScale
    ) {
        m_startPosition = startPos;
        m_startRotation = startRot;
        m_startScale = startScale;
        
        m_endPosition = endPos;
        m_endRotation = endRot;
        m_endScale = endScale;
        
        m_currentPosition = startPos;
        m_currentRotation = startRot;
        m_currentScale = startScale;
        
        m_hasAnimation = true;
        m_state = State::AT_START;
    }

    // Load animation keyframes directly from Assimp scene
    bool loadFromScene(const aiScene* scene) {
        if (!scene || scene->mNumAnimations == 0) {
            std::cout << "[NodeAnimation] No animations found in scene" << std::endl;
            return false;
        }

        aiAnimation* anim = scene->mAnimations[0];
        
        // Find the first channel with position keyframes
        for (unsigned int i = 0; i < anim->mNumChannels; i++) {
            aiNodeAnim* channel = anim->mChannels[i];
            
            if (channel->mNumPositionKeys >= 2) {
                // Extract start keyframe (first)
                const auto& posStart = channel->mPositionKeys[0].mValue;
                m_startPosition = glm::vec3(posStart.x, posStart.y, posStart.z);
                
                // Extract end keyframe (last)
                int lastIdx = channel->mNumPositionKeys - 1;
                const auto& posEnd = channel->mPositionKeys[lastIdx].mValue;
                m_endPosition = glm::vec3(posEnd.x, posEnd.y, posEnd.z);

                // Extract rotation if available
                if (channel->mNumRotationKeys >= 1) {
                    const auto& rotStart = channel->mRotationKeys[0].mValue;
                    m_startRotation = glm::quat(rotStart.w, rotStart.x, rotStart.y, rotStart.z);
                    
                    int lastRotIdx = channel->mNumRotationKeys - 1;
                    const auto& rotEnd = channel->mRotationKeys[lastRotIdx].mValue;
                    m_endRotation = glm::quat(rotEnd.w, rotEnd.x, rotEnd.y, rotEnd.z);
                }

                // Extract scale if available
                if (channel->mNumScalingKeys >= 1) {
                    const auto& scaleStart = channel->mScalingKeys[0].mValue;
                    m_startScale = glm::vec3(scaleStart.x, scaleStart.y, scaleStart.z);
                    
                    int lastScaleIdx = channel->mNumScalingKeys - 1;
                    const auto& scaleEnd = channel->mScalingKeys[lastScaleIdx].mValue;
                    m_endScale = glm::vec3(scaleEnd.x, scaleEnd.y, scaleEnd.z);
                }

                // Initialize current values to start
                m_currentPosition = m_startPosition;
                m_currentRotation = m_startRotation;
                m_currentScale = m_startScale;
                m_hasAnimation = true;
                m_state = State::AT_START;

                std::cout << "[NodeAnimation] Loaded from channel: " << channel->mNodeName.C_Str() << std::endl;
                std::cout << "  Start pos: (" << m_startPosition.x << ", " << m_startPosition.y << ", " << m_startPosition.z << ")" << std::endl;
                std::cout << "  End pos: (" << m_endPosition.x << ", " << m_endPosition.y << ", " << m_endPosition.z << ")" << std::endl;
                
                return true;
            }
        }

        std::cout << "[NodeAnimation] No position keyframes found" << std::endl;
        return false;
    }

    // Trigger animation to end position (call on left-click)
    void trigger() {
        if (!m_hasAnimation) return;
        
        if (m_state == State::AT_START) {
            m_state = State::ANIMATING_TO_END;
            m_currentTime = 0.0f;
            std::cout << "[NodeAnimation] Triggered: moving to end position" << std::endl;
        }
    }

    // Trigger animation back to start position (call on right-click or toggle)
    void triggerReturn() {
        if (!m_hasAnimation) return;
        
        if (m_state == State::AT_END) {
            m_state = State::ANIMATING_TO_START;
            m_currentTime = 0.0f;
            std::cout << "[NodeAnimation] Triggered: returning to start position" << std::endl;
        }
    }

    // Toggle between start and end (convenience method)
    void toggle() {
        if (m_state == State::AT_START) {
            trigger();
        } else if (m_state == State::AT_END) {
            triggerReturn();
        }
    }

    // Update animation (call every frame)
    void update(float deltaTime) {
        if (!m_hasAnimation || !isAnimating()) return;

        m_currentTime += deltaTime;
        float t = glm::clamp(m_currentTime / m_duration, 0.0f, 1.0f);

        // Smooth ease in-out interpolation
        t = smoothstep(t);

        // Determine interpolation direction
        glm::vec3 fromPos, toPos;
        glm::quat fromRot, toRot;
        glm::vec3 fromScale, toScale;

        if (m_state == State::ANIMATING_TO_END) {
            fromPos = m_startPosition;  toPos = m_endPosition;
            fromRot = m_startRotation;  toRot = m_endRotation;
            fromScale = m_startScale;   toScale = m_endScale;
        } else {
            fromPos = m_endPosition;    toPos = m_startPosition;
            fromRot = m_endRotation;    toRot = m_startRotation;
            fromScale = m_endScale;     toScale = m_startScale;
        }

        // Interpolate
        m_currentPosition = glm::mix(fromPos, toPos, t);
        m_currentRotation = glm::slerp(fromRot, toRot, t);
        m_currentScale = glm::mix(fromScale, toScale, t);

        // Check if animation completed
        if (t >= 1.0f) {
            if (m_state == State::ANIMATING_TO_END) {
                m_state = State::AT_END;
                m_currentPosition = m_endPosition;
                m_currentRotation = m_endRotation;
                m_currentScale = m_endScale;
            } else {
                m_state = State::AT_START;
                m_currentPosition = m_startPosition;
                m_currentRotation = m_startRotation;
                m_currentScale = m_startScale;
            }
            std::cout << "[NodeAnimation] Animation complete" << std::endl;
        }
    }

    // Get current transform matrix (apply to model matrix)
    glm::mat4 getTransform() const {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, m_currentPosition);
        transform = transform * glm::mat4_cast(m_currentRotation);
        transform = glm::scale(transform, m_currentScale);
        return transform;
    }

    // Get individual components
    glm::vec3 getCurrentPosition() const { return m_currentPosition; }
    glm::quat getCurrentRotation() const { return m_currentRotation; }
    glm::vec3 getCurrentScale() const { return m_currentScale; }

    // Reset to start position immediately
    void reset() {
        m_currentPosition = m_startPosition;
        m_currentRotation = m_startRotation;
        m_currentScale = m_startScale;
        m_state = State::AT_START;
        m_currentTime = 0.0f;
    }

private:
    // Smooth interpolation function (ease in-out)
    static float smoothstep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }
};
