#ifndef PIXIE_EXPORTS_H
#define PIXIE_EXPORTS_H

#ifdef _WIN32
    #ifndef PIXIE_API
        #ifdef PIXIE_DLL_EXPORTS
            /* We are building this library */
            #define PIXIE_API       __declspec(dllexport)
            #define PIXIE_EXPORT    __declspec(dllexport)
        #else
            /* We are using this library */
            #define PIXIE_API       __declspec(dllimport)
            #define PIXIE_EXPORT
        #endif // PIXIE_DLL_EXPORTS
    #endif // PIXIE_API
#else
    #define PIXIE_API
    #define PIXIE_EXPORT
#endif // _WIN32

#endif /* PIXIE_EXPORTS_H */
