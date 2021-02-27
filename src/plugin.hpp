#ifdef _WIN32
#define EXPORT_DLL __declspec( dllexport )
#else
#define EXPORT_DLL
#endif

extern "C" {
    EXPORT_DLL bool initialize(void);
    EXPORT_DLL const char* get_error(void);
}
