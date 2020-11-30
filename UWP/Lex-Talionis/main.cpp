#include <string>
#include <iostream>
#include <memory>

#include "SDL.h"

#include "Python.h"
#include <ppltasks.h>

#include <exception>

#include <eh.h>

#include <codecvt>
#include <locale> 

#include <iostream>
#include <fstream>

std::wstring stringToWstring(const char* utf8Bytes)
{
    //setup converter
    using convert_type = std::codecvt_utf8<typename std::wstring::value_type>;
    std::wstring_convert<convert_type, typename std::wstring::value_type> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.from_bytes(utf8Bytes);
}


extern "C" static PyObject *
write_to_prefs(PyObject * self, PyObject * args)
{

    std::vector<PyObject*> objects;
    int argc = PyTuple_GET_SIZE(args);

    if (argc != 2) {
        Py_RETURN_NONE;
    }

    objects.resize(argc);

    for (int i = 0; i < argc; i++) {
        objects[i] = PyTuple_GET_ITEM(args, i);
    }


    auto file_data = PyUnicode_AsUTF8(objects[0]);
    auto content_data = PyUnicode_AsUTF8(objects[1]);

    /*
    auto filePathW = stringToWstring(file_data);
    auto fileContentW = stringToWstring(content_data);

    auto filePath = ref new Platform::String(filePathW.c_str());
    auto fileContent = ref new Platform::String(fileContentW.c_str());

    auto localFolder = Windows::Storage::ApplicationData::Current->LocalFolder;

    auto collisionOpts = Windows::Storage::CreationCollisionOption::ReplaceExisting;

    auto localFile =
        localFolder->CreateFileAsync(ref new Platform::String(L"File.txt"), collisionOpts)->GetResults();

    Windows::Storage::FileIO::WriteTextAsync(localFile, fileContent)->GetResults();
    */

    auto content = std::string(content_data);

    char* prefpath = SDL_GetPrefPath("data", "lex-talionis");
    auto _tryPath = std::string(prefpath) + std::string(file_data);

    std::ofstream myfile;
    myfile.open(_tryPath);
    myfile << content;
    myfile.close();

    SDL_free(prefpath);


    Py_RETURN_NONE;
}
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
    {"write_to_prefs", write_to_prefs,
     METH_VARARGS, NULL},
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

ref class back_handler sealed {
public:

    back_handler() {

    }

    void App_BackRequested(
        Platform::Object^ sender,
        Windows::UI::Core::BackRequestedEventArgs^ e)
    {
        // Navigate back if possible, and if the event has not
        // already been handled.
        if (e->Handled == false)
        {
            e->Handled = true;
        }
    }

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

    // Be sure to enable "Yes with SEH Exceptions (/EHa)" in C++ / Code Generation;
    /*_set_se_translator([](unsigned int u, EXCEPTION_POINTERS* pExp) {
        std::string error = "SE Exception: ";
        switch (u) {
        case 0xC0000005:
            error += "Access Violation";
            break;
        default:
            char result[11];
            sprintf_s(result, 11, "0x%08X", u);
            error += result;
        };
        throw std::exception(error.c_str());
        });*/

    auto python_shell = std::make_shared<shell>();

    auto button_handler = ref new back_handler();

    Windows::UI::Core::SystemNavigationManager::GetForCurrentView()->
        BackRequested += ref new Windows::Foundation::EventHandler<
        Windows::UI::Core::BackRequestedEventArgs^>(
            button_handler, &back_handler::App_BackRequested);


    python_shell->initialize();

    auto lex_talionis = readFile("Assets/Lex-Talionis/main.py");


    /*typedef void (*SignalHandlerPointer)(int);

    SignalHandlerPointer previousHandler;
    previousHandler = signal(SIGSEGV, SignalHandler);*/
    //try 
    {

        PyEval_InitThreads();

        python_shell->set_metro(PyImport_ImportModule("metrosetup"));

        auto module = PyImport_ImportModule("faulthandler");
        if (module == NULL) {
            return -1;
        }

        _Py_IDENTIFIER(enable);

        auto res = _PyObject_CallMethodId(module, &PyId_enable, "");
        Py_DECREF(module);
        if (res == NULL) {
            PyErr_Print();
            return -1;
        }
            
        Py_DECREF(res);

        
        python_shell->run_string(lex_talionis);

        PyEval_ReleaseThread(PyThreadState_Get());

        
    }
    /*catch (...)
    {
        PyErr_Print();

        printf("Exception Caught:");
    }*/

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

