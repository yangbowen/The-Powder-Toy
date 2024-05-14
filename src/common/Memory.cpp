#include <mimalloc-new-delete.h>

#include <mimalloc.h>

#ifdef _WIN32_WINNT
extern "C" void __stdcall OutputDebugStringA(const char*);
namespace {
	struct preinit_t {
		preinit_t() {
			mi_register_output(
				[](const char* message, void*) {
					OutputDebugStringA(message);
				},
				nullptr
			);
		}
	} preinit;
}
#endif
