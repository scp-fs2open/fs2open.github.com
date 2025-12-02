#!/usr/bin/env bash

# Helper script to run the Vulkan build and collect logs for debugging.
# Intended usage: from the FSO repo, run:
#   ./scripts/vulkan_debug_session.sh [extra FSO args...]
# It will:
#   - Run the Vulkan exe (Debug) once
#   - Capture stdout/stderr
#   - Snapshot fs2_open.log and Vulkan debug logs
#   - Store everything under out/vulkan_debug/<timestamp>/

set -euo pipefail

if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
    echo "Runs the Vulkan build and collects logs into out/vulkan_debug/<timestamp>/"
    echo "Usage: ./scripts/vulkan_debug_session.sh [fso_arguments]*"
    echo
    echo "Environment variables:"
    echo "  FSO_EXE  Path to the FSO executable (default: build/bin/Debug/fs2_26_0_0.exe)"
    exit 0
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
FSO_REPO="$(readlink -f "${SCRIPT_DIR}/..")"
cd "${FSO_REPO}"

FSO_EXE_REL="${FSO_EXE:-build/bin/Debug/fs2_26_0_0.exe}"
FSO_EXE_PATH="$(readlink -f "${FSO_EXE_REL}")"

if [[ ! -x "${FSO_EXE_PATH}" ]]; then
    echo "Error: FSO executable not found or not executable:"
    echo "  ${FSO_EXE_PATH}"
    echo "Build the Debug Vulkan target first, or set FSO_EXE to the correct path."
    exit 1
fi

SESSION_ROOT="${FSO_REPO}/out/vulkan_debug"
mkdir -p "${SESSION_ROOT}"

SESSION_ID="$(date +%Y%m%d_%H%M%S)"
SESSION_DIR="${SESSION_ROOT}/${SESSION_ID}"
mkdir -p "${SESSION_DIR}"

echo "Vulkan debug session: ${SESSION_ID}"
echo "Session directory: ${SESSION_DIR}"
echo

detect_fs2_data_dir() {
    local data_dir=""

    # WSL / Windows path (fs2_open.log lives under %APPDATA%)
    if command -v wslpath >/dev/null 2>&1 && command -v cmd.exe >/dev/null 2>&1; then
        local win_appdata
        win_appdata="$(cmd.exe /C "echo %APPDATA%" | tr -d '\r')"
        if [[ -n "${win_appdata}" ]]; then
            data_dir="$(wslpath -u "${win_appdata}")/HardLightProductions/FreeSpaceOpen/data"
        fi
    fi

    # Fallback: typical Linux path
    if [[ -z "${data_dir}" ]]; then
        data_dir="${HOME}/.local/share/HardLightProductions/FreeSpaceOpen/data"
    fi

    echo "${data_dir}"
}

prepare_log_file() {
    local label="$1"
    local path="$2"

    if [[ -f "${path}" ]]; then
        echo "  Found existing ${label} at ${path}, snapshotting and truncating..."
        cp "${path}" "${SESSION_DIR}/${label}_pre.log"
        : > "${path}"
    fi
}

FS2_DATA_DIR="$(detect_fs2_data_dir)"
FS2_LOG_PATH="${FS2_DATA_DIR}/fs2_open.log"

GAME_DIR="${FSO_REPO}"
VK_LOG_PATH="${GAME_DIR}/vulkan_debug.log"
VK_HDR_LOG_PATH="${GAME_DIR}/vulkan_hdr_debug.txt"

echo "Preparing log files..."
prepare_log_file "fs2_open" "${FS2_LOG_PATH}"
prepare_log_file "vulkan_debug" "${VK_LOG_PATH}"
prepare_log_file "vulkan_hdr_debug" "${VK_HDR_LOG_PATH}"

echo
echo "Running FSO Vulkan build once. Reproduce your issue, then quit the game."
echo "Executable: ${FSO_EXE_PATH}"
echo "Arguments : -vulkan -window $*"
echo

{
    echo "# Vulkan debug session ${SESSION_ID}"
    echo "cwd: ${FSO_REPO}"
    echo "exe: ${FSO_EXE_PATH}"
    echo "args: -vulkan -window $*"
    echo
} > "${SESSION_DIR}/command.txt"

set +e
"${FSO_EXE_PATH}" -vulkan -window "$@" > "${SESSION_DIR}/stdout_stderr.txt" 2>&1
EXIT_CODE=$?
set -e

echo
echo "FSO exited with code ${EXIT_CODE}"
echo "Collecting logs into session directory..."

if [[ -f "${FS2_LOG_PATH}" ]]; then
    cp "${FS2_LOG_PATH}" "${SESSION_DIR}/fs2_open.log"
else
    echo "  Warning: fs2_open.log not found at ${FS2_LOG_PATH}"
fi

if [[ -f "${VK_LOG_PATH}" ]]; then
    cp "${VK_LOG_PATH}" "${SESSION_DIR}/vulkan_debug.log"
else
    echo "  Warning: vulkan_debug.log not found at ${VK_LOG_PATH}"
fi

if [[ -f "${VK_HDR_LOG_PATH}" ]]; then
    cp "${VK_HDR_LOG_PATH}" "${SESSION_DIR}/vulkan_hdr_debug.txt"
fi

echo
echo "Done."
echo "For LLM debugging, point to this directory:"
echo "  ${SESSION_DIR}"
echo "and especially share:"
echo "  - stdout_stderr.txt"
echo "  - fs2_open.log"
echo "  - vulkan_debug.log"

