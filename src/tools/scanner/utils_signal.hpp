#ifndef UTILS_SIGNAL_HPP
#define UTILS_SIGNAL_HPP

#include <atomic>
#include <setjmp.h>
#include <csignal> // For signal constants

namespace FWA::SCANNER
{
	// --- Signal Handling ---

	/**
	 * @brief Signal handler for SIGSEGV (segmentation fault).
	 * Sets a global flag and uses longjmp to return to a saved context (setjmp in safeReadQuadlet).
	 *
	 * @param signal The signal number (expected to be SIGSEGV).
	 */
	void segfaultHandler(int signal);

	// Global variables for signal handling (defined in utils.cpp)
	extern std::atomic<bool> g_segfaultOccurred;
	extern jmp_buf g_jmpBuf;

} // namespace FWA::SCANNER

#endif // UTILS_SIGNAL_HPP