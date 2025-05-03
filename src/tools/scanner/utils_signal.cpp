#include "utils_signal.hpp"

#include <iostream>
#include <csignal> // For signal constants
#include <setjmp.h>
#include <atomic>

namespace FWA::SCANNER
{
	// --- Global variables for signal handling ---
	std::atomic<bool> g_segfaultOccurred(false);
	jmp_buf g_jmpBuf;

	// --- Signal Handling Implementation ---

	void segfaultHandler(int signal)
	{
		if (signal == SIGSEGV)
		{
			std::cerr << "Caught segmentation fault (SIGSEGV)! Recovering..." << std::endl;
			g_segfaultOccurred = true;
			// Jump back to the saved context in safeReadQuadlet
			longjmp(g_jmpBuf, 1);
		}
		else
		{
			std::cerr << "Caught unexpected signal: " << signal << std::endl;
			// Optionally re-raise or exit
			std::abort();
		}
	}

} // namespace FWA::SCANNER