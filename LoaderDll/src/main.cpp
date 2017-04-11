#include <Windows.h>
#include <utils.hpp>
#include <cstdint>
#include <thread>
#include <string>
#include <atomic>

#pragma warning(push, 1)
#include <PolyHook.hpp>
#pragma warning(pop)

namespace mono
{
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoProperty MonoProperty;
	typedef struct _MonoEvent MonoEvent;
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoAssembly MonoAssembly;

	typedef enum
	{
		MONO_IMAGE_OK,
		MONO_IMAGE_ERROR_ERRNO,
		MONO_IMAGE_MISSING_ASSEMBLYREF,
		MONO_IMAGE_IMAGE_INVALID
	} MonoImageOpenStatus;

	typedef MonoClass*    (__cdecl* mono_class_from_name_t)(MonoImage* image, const char* name_space, const char* name);
	typedef MonoMethod*   (__cdecl* mono_class_get_method_from_name_t)(MonoClass* mclass, const char* name, int param_count);
	typedef MonoObject*   (__cdecl* mono_runtime_invoke_t)(MonoMethod* method, void* obj, void** params, MonoObject** exc);
	typedef MonoProperty* (__cdecl* mono_class_get_property_from_name_t)(MonoClass* mclass, const char* name);
	typedef MonoMethod*   (__cdecl* mono_property_get_set_method_t)(MonoProperty* prop);
	typedef MonoMethod*   (__cdecl* mono_property_get_get_method_t)(MonoProperty* prop);
	typedef MonoDomain*   (__cdecl* mono_domain_get_t)(void);
	typedef MonoAssembly* (__cdecl* mono_assembly_open_t)(const char *filename, MonoImageOpenStatus *status);
	typedef MonoAssembly* (__cdecl* mono_domain_assembly_open_t)(MonoDomain* domain, const char *filename);
	typedef const char*   (__cdecl* mono_image_strerror_t)(MonoImageOpenStatus status);
	typedef MonoImage*    (__cdecl* mono_assembly_get_image_t)(MonoAssembly *assembly);

	mono_class_from_name_t              class_from_name = nullptr;
	mono_class_get_method_from_name_t   class_get_method_from_name = nullptr;
	mono_runtime_invoke_t               runtime_invoke = nullptr;
	mono_class_get_property_from_name_t class_get_property_from_name = nullptr;
	mono_property_get_set_method_t      property_get_set_method = nullptr;
	mono_property_get_get_method_t      property_get_get_method = nullptr;
	mono_domain_get_t                   domain_get = nullptr;
	mono_assembly_open_t                assembly_open = nullptr;
	mono_image_strerror_t               image_strerror = nullptr;
	mono_assembly_get_image_t           assembly_get_image = nullptr;
	mono_domain_assembly_open_t         domain_assembly_open = nullptr;
	bool get_exports()
	{
		auto mono = GetModuleHandle(TEXT("mono.dll"));

		auto get_mono_export = [mono](auto name) {
			return GetProcAddress(mono, name);
		};

		class_from_name              = (mono_class_from_name_t)             get_mono_export("mono_class_from_name");
		class_get_method_from_name   = (mono_class_get_method_from_name_t)  get_mono_export("mono_class_get_method_from_name");
		runtime_invoke               = (mono_runtime_invoke_t)              get_mono_export("mono_runtime_invoke");
		class_get_property_from_name = (mono_class_get_property_from_name_t)get_mono_export("mono_class_get_property_from_name");
		property_get_set_method      = (mono_property_get_set_method_t)     get_mono_export("mono_property_get_set_method");
		property_get_get_method      = (mono_property_get_get_method_t)     get_mono_export("mono_property_get_get_method");
		domain_get                   = (mono_domain_get_t)                  get_mono_export("mono_domain_get");
		assembly_open                = (mono_assembly_open_t)               get_mono_export("mono_assembly_open");
		image_strerror               = (mono_image_strerror_t)              get_mono_export("mono_image_strerror");
		assembly_get_image           = (mono_assembly_get_image_t)          get_mono_export("mono_assembly_get_image");
		domain_assembly_open         = (mono_domain_assembly_open_t)        get_mono_export("mono_domain_assembly_open");

		return class_from_name &&
			class_get_method_from_name &&
			runtime_invoke &&
			class_get_property_from_name &&
			property_get_set_method &&
			property_get_get_method &&
			domain_get &&
			assembly_open &&
			image_strerror &&
			assembly_get_image && 
			domain_assembly_open;
	}
}

PLH::Detour mono_domain_get_detour;
std::string loader_folder;
bool load_finished = false;

void load_script(mono::MonoDomain* domain, const std::string& path)
{
	auto fullpath = loader_folder + "\\" + path;

	auto assembly = mono::domain_assembly_open(domain, fullpath.data());

	if(!assembly) {
		utils::console_print("[load_script] mono_assembly_open failed\n");
		return;
	}

	auto image = mono::assembly_get_image(assembly);
	auto main_class = mono::class_from_name(image, "BallisticOverkill", "Script");

	if(!main_class) {
		utils::console_print("[load_script] mono_class_from_name failed.\n");
		return;
	}

	auto script_entry = mono::class_get_method_from_name(main_class, "Main", 0);

	if(!script_entry) {
		utils::console_print("[load_script] Script does not have a \"Main\" method.\n");
		return;
	}

	mono::runtime_invoke(script_entry, nullptr, nullptr, nullptr);
}

mono::MonoDomain* __cdecl hk_mono_domain_get(void)
{
	static std::atomic_bool once = false;
	static auto orig = mono_domain_get_detour.GetOriginal<mono::mono_domain_get_t>();

	auto domain = orig();

	if(!once) {
		once = true;
		load_script(domain, "BallisticOverkill.dll");
		load_finished = true;
	}

	return domain;
}

bool on_dll_attach(std::uintptr_t base)
{
	utils::attach_console();

	try {
		if(utils::wait_for_modules(10000, "mono.dll") == WAIT_TIMEOUT)
			throw std::runtime_error("mono.dll did not load in time.");

		char buf[MAX_PATH];
		if(GetModuleFileNameA(reinterpret_cast<HMODULE>(base), buf, MAX_PATH)) {
			loader_folder = std::string(buf);
			loader_folder = loader_folder.erase(loader_folder.find_last_of('\\'));
		} else {
			throw std::runtime_error("Could not find loader dir.");
		}

		utils::console_print("[on_dll_attach] Loader Directory: %s\n", loader_folder.data());

		mono::get_exports();

		if(mono::domain_get) {
			mono_domain_get_detour.SetupHook(mono::domain_get, hk_mono_domain_get);
			mono_domain_get_detour.Hook();
		}

		while(!load_finished)
			Sleep(500);

		FreeLibraryAndExitThread((HMODULE)base, 1);
		return 1;
	} catch(const std::exception& ex) {
		utils::console_print("An error occured during initialization.\nError: %s\n", ex.what());
		utils::console_print("Press any key to exit\n");
		utils::console_read_key();
		FreeLibraryAndExitThread((HMODULE)base, 1);
		return 0;
	}
}
bool on_dll_detach(bool exiting)
{
	if(!exiting) {
		mono_domain_get_detour.UnHook();
		utils::detach_console();
	}
	return 1;
}

BOOL WINAPI DllMain(
	_In_      HINSTANCE hinstDll,
	_In_      DWORD     fdwReason,
	_In_opt_	LPVOID    lpvReserved
)
{
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hinstDll);
			CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)on_dll_attach, hinstDll, 0, nullptr);
		case DLL_PROCESS_DETACH:
			return on_dll_detach(lpvReserved != nullptr);
	}
	return TRUE;
}