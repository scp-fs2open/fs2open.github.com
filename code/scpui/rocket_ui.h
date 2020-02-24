#pragma once

// Forward definition
namespace Rocket {
namespace Core {
class Context;
}
} // namespace Rocket

/**
 * @brief Contains all functions and types related to the libRocket based UI system
 */
namespace scpui {

/**
 * @brief Initialzes libRocket and the associated interfaces
 */
void initialize();

/**
 * @brief Sets an offset by which the libRocket interface is moved by.
 *
 * @details This translates all drawing operations by the specified offset and transforms input events by the same
 * offset to bring them back into the same frame of reference for libRocket.
 *
 * @param x The x-offset
 * @param y The y-offset
 */
void setOffset(float x, float y);

/**
 * @brief Reloads all libRocket contexts currently active
 *
 * This can be used to reload the active documents if the files contained documents.
 */
void reloadAllContexts();

/**
 * @brief Enables mouse and text input on the specified input
 *
 * If there was another context with input enabled then input will be disabled for that context before enabling input
 * on this context.
 *
 * @param main_ctx The context ot enable input for
 */
void enableInput(Rocket::Core::Context* main_ctx);

/**
 * @brief Disable all input handling
 */
void disableInput();

/**
 * @brief Shutdown the scripting part of libRocket
 *
 * This needs to be called before the scripting state is destroyed so that all resources are deallocated properly.
 */
void shutdown_scripting();

/**
 * @brief Frees all resources of the UI system
 */
void shutdown();

} // namespace scpui
