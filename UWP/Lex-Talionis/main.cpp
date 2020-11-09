#include <string>
#include <iostream>
#include <memory>

#include "SDL.h"

#include "Python.h"
#include <ppltasks.h>


extern "C" static PyObject *
add_to_stdout(PyObject * self, PyObject * args)
{
    Py_UNICODE* data;
    if (!PyArg_ParseTuple(args, "u", &data))
        return NULL;
    auto paraString = data;
    auto formattedText = std::wstring(paraString).append(L"\r\n");
    OutputDebugString(formattedText.c_str());

    Py_RETURN_NONE;
}

extern "C" static PyObject *
add_to_stderr(PyObject * self, PyObject * args)
{
    Py_UNICODE* data;
    if (!PyArg_ParseTuple(args, "u", &data))
        return NULL;
    auto paraString = data;
    auto formattedText = std::wstring(paraString).append(L"\r\n");
    OutputDebugString(formattedText.c_str());
    Py_RETURN_NONE;
}

extern "C" static PyObject *
readline(PyObject * self, PyObject * args)
{
    PyErr_SetString(PyExc_IOError, "Getting input from console is not implemented yet");
    return NULL;
}

extern "C" static PyObject *
metroui_exit(PyObject * self, PyObject * args)
{
    exit(0);
    Py_RETURN_NONE;
}

static struct PyMethodDef metroui_methods[] = {
    {"add_to_stdout", add_to_stdout,
     METH_VARARGS, NULL},
    {"add_to_stderr", add_to_stderr,
     METH_VARARGS, NULL},
    {"readline", readline, METH_NOARGS, NULL},
    {"exit", metroui_exit,
     METH_NOARGS, NULL},
  {NULL, NULL}
};


static struct PyModuleDef metroui = {
    PyModuleDef_HEAD_INIT,
    "metroui",
    NULL,
    -1,
    metroui_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

static PyObject*
PyInit_metroui()
{
    return PyModule_Create(&metroui);
}

class shell {
public:

    shell() {

        PyImport_AppendInittab("metroui", PyInit_metroui);

        /* compute python path */
        Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
        Windows::Storage::StorageFolder^ installedLocation = package->InstalledLocation;
        wcscpy_s(progpath, installedLocation->Path->Data());


        /*
        std::wstring python_path = std::wstring(Py_GetPath());
        std::wstring assets_path = std::wstring(progpath) + L"/Assets/Lex-Talionis";

        std::wstring joined = python_path + L";" + assets_path;

        Py_SetPath(joined.c_str());
        */

        /* XXX how to determine executable name? */
        wcscat_s(progpath, L"\\Lex-Talionis.exe");
        Py_SetProgramName(progpath);
        // Continue when loaded

    }

    void initialize() {

        Py_Initialize();

    }

    void deinitialize() {
        //PyGILState_STATE s = PyGILState_Ensure();
        Py_Finalize();
    }

    PyObject* try_compile(const std::wstring& string_code)
    {
        //PyGILState_STATE s = PyGILState_Ensure();
        PyObject* code = PyObject_CallMethod(metrosetup, "compile", "u", string_code.c_str());
        if (code == NULL) {
            PyErr_Print();
            std::cout << "Could not compile code" << std::endl;
        }
        else if (code == Py_None) {
            Py_DECREF(code);
        }

        //PyGILState_Release(s);
        return code;
    }

    void run_code(PyObject* code)
    {
        //PyGILState_STATE s = PyGILState_Ensure();
        PyObject* result = PyObject_CallMethod(metrosetup, "eval", "O", code);
        if (result == NULL) {
            PyErr_Print();
        }
        else {
            Py_DECREF(result);
        }
        Py_CLEAR(code);
        //PyGILState_Release(s);
    }

    void run_string(const std::wstring& code) {
        std::string str(code.begin(), code.end());
        PyRun_SimpleString(str.c_str());
    }

    void set_metro(PyObject* metro) {
        metrosetup = metro;

        if (metrosetup == NULL) {
            PyErr_Print();
        }
    }

private:
    PyObject* metrosetup;
    wchar_t progpath[1024];

};

#include <sstream>
#include <fstream>
#include <codecvt>

#include <signal.h>

std::wstring readFile(const char* filename)
{
    std::wifstream wif(filename);
    wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << wif.rdbuf();
    return wss.str();
}

void SignalHandler(int signal)
{
    printf("Signal %d", signal);
    throw "!Access Violation!";
}

int main(int argc, char** argv)

{
    auto python_shell = std::make_shared<shell>();

    python_shell->initialize();

    auto lex_talionis = readFile("Assets/Lex-Talionis/main.py");


    typedef void (*SignalHandlerPointer)(int);

    SignalHandlerPointer previousHandler;
    previousHandler = signal(SIGSEGV, SignalHandler);
    try {

        PyEval_InitThreads();

        python_shell->set_metro(PyImport_ImportModule("metrosetup"));
        
        python_shell->run_string(lex_talionis);

        PyEval_ReleaseThread(PyThreadState_Get());

        
    }
    catch (char* e)
    {
        PyErr_PrintEx(0);

        printf("Exception Caught: %s\n", e);
    }

    //PyEval_ReleaseThread(PyThreadState_Get());
    

    python_shell->deinitialize();

    return 0;

    /*
    SDL_DisplayMode mode;

    SDL_Window* window = NULL;

    SDL_Renderer* renderer = NULL;

    SDL_Event evt;



    if (SDL_Init(SDL_INIT_VIDEO) != 0) {

        return 1;

    }



    if (SDL_GetCurrentDisplayMode(0, &mode) != 0) {

        return 1;

    }



    if (SDL_CreateWindowAndRenderer(mode.w, mode.h, SDL_WINDOW_RESIZABLE, &window, &renderer) != 0) {

        return 1;

    }



    while (1) {

        while (SDL_PollEvent(&evt)) {

        }


        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // line of code in question
        SDL_RenderClear(renderer);


        SDL_RenderPresent(renderer);

    }*/

}

