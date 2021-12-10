//
// PyShell.xaml.cpp
// Implementation of the PyShell.xaml class.
//

#include "pch.h"
#include "PyShell.h"
#include "Python.h"
#include <ppltasks.h>

using namespace PythonWinRT;

using namespace Platform;
using namespace Platform::Collections;

using namespace Concurrency;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::ApplicationSettings;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;

static PyShell^ singleton = nullptr;

#include <codecvt>
#include <locale> 

#include <iostream>
#include <fstream>

//delegate void OutputHandler(String^ s);

 

String^ UTFCharsToString(char* chars)
{
    

    size_t newsize = strlen(chars) + 1;
    wchar_t* wcstring = new wchar_t[newsize];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wcstring, newsize, chars, _TRUNCATE);
    String^ str = ref new Platform::String(wcstring);
    delete[] wcstring;
    return str;
}

String^ UTFCharsToString(wchar_t* chars)
{
    char* chars2 = (char*)chars;

    size_t newsize = strlen(chars2) + 1;
    wchar_t* wcstring = new wchar_t[newsize];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wcstring, newsize, chars2, _TRUNCATE);
    String^ str = ref new Platform::String(wcstring);
    delete[] wcstring;
    return str;
}

std::string PlatformStringToStdString(String^ s) {

    //return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s->Data());

    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s->Data());

}

String^ UTFPythonStringToString(PyObject* s) {


    if (s == NULL) return nullptr;

    return UTFCharsToString(PyUnicode_AsUTF8(s));

}


std::wstring UtfCharsToWstring(const char* utf8Bytes)
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

    /*char* prefpath = SDL_GetPrefPath("games", "lex-talionis");
    auto _tryPath = std::string(prefpath) + std::string("file.txt");

    std::ofstream myfile;
    myfile.open(_tryPath);
    myfile << "Writing this to a file.\n";
    myfile.close();


    SDL_free(prefpath);*/


    Py_RETURN_NONE;
}

extern "C" static PyObject *
add_to_stdout(PyObject *self, PyObject *args)
{
    Py_UNICODE *data;
    if (!PyArg_ParseTuple(args, "u", &data))
        return NULL; 
      
    
    singleton->addOut(data);
   
    OutputDebugString(data);

    Py_RETURN_NONE;
}





extern "C" static PyObject *
add_to_stderr(PyObject *self, PyObject *args)
{
    Py_UNICODE *data;
    if (!PyArg_ParseTuple(args, "u", &data))
        return NULL;
    
       
    singleton->addError(data);

    Py_RETURN_NONE;
}

extern "C" static PyObject *
readline(PyObject *self, PyObject *args)
{
    PyErr_SetString(PyExc_IOError, "Getting input from console is not implemented yet");
    return NULL;
}

extern "C" static PyObject *
metroui_exit(PyObject *self, PyObject *args)
{
    //singleton->exit();
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





static wchar_t progpath[1024];
static std::wstring proghome;
PyShell::PyShell()
{
    PyImport_AppendInittab("metroui", PyInit_metroui);

    Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
    Windows::Storage::StorageFolder^ installedLocation = package->InstalledLocation;
    wcscpy_s(progpath, installedLocation->Path->Data());
    
    proghome = std::wstring(progpath);

    Py_SetPythonHome((wchar_t*)proghome.c_str());

    /* XXX how to determine executable name? */
    wcscat_s(progpath, L"\\PythonWinRT.exe");
    Py_SetProgramName(progpath);

    //singleton = this;

}



void PyShell::addOut(Py_UNICODE* m) {
    ConsoleOutputEvent(ref new String(m));
}

void PyShell::addError(Py_UNICODE* m) {
    ErrorOutputEvent(ref new String(m));
}

void PyShell::addDebug(Py_UNICODE* m) {
    DebugOutputEvent(ref new String(m));
}
void PyShell::echoOut(Py_UNICODE* m) {
    EchoOutputEvent(ref new String(m));
}

void PyShell::echoOut(String^ m) {
    EchoOutputEvent(m);
}

void PyShell::StartInterpreter()
{

    Py_Initialize();
    //PyEval_InitThreads();
    
    /* boot interactive shell */
    metrosetup = PyImport_ImportModule("metrosetup");
    if (metrosetup == NULL) {
        PyErr_Print();
    }

}

void PyShell::StopInterpreter()
{
    Py_Finalize();
}

void PyShell::RunString(const std::string& data)
{
    PyRun_SimpleString(data.c_str());
}





void PyShell::RunSimpleString(String^ data) {

    
    std::string utf8 = PlatformStringToStdString(data);
        
        //std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(data->Data());
    echoOut(data);
    PyRun_SimpleString(utf8.c_str());


}





void PyShell::ImportModule(String^ pmoduleName) {

    std::string moduleName = PlatformStringToStdString(pmoduleName);

    PyObject* module = PyImport_ImportModule(moduleName.c_str());    

    if (module == NULL) {
        PyErr_Print();
    }

}

String^ PyShell::ExecuteStringFunc(String^ pmoduleName, String^ pfuncName, String^ pparam) {

    PyObject*  pModule,  * pFunc, * pythonArgument, * pValue;


    std::string moduleName = PlatformStringToStdString(pmoduleName);
    std::string funcName = PlatformStringToStdString(pfuncName);
    std::string param = PlatformStringToStdString(pparam);


    
    pModule = PyImport_ImportModule(moduleName.c_str());

    if (pModule == NULL) return nullptr;

    pythonArgument = PyTuple_New(1);
    pValue = PyUnicode_FromString(param.c_str());
    
    PyTuple_SetItem(pythonArgument, 0, pValue);
    

    pFunc = PyObject_GetAttrString(pModule, funcName.c_str());
    
    if (pFunc == NULL) return nullptr;

    PyObject* result = PyObject_CallObject(pFunc, pythonArgument);

    Py_DECREF(pValue);Py_DECREF(pythonArgument); Py_DECREF(pFunc); Py_DECREF(pModule);

    String^ r = UTFPythonStringToString(result);

   
    Py_DECREF(result); 

    return r;

}


PyShell^ RTPython::GetShell(void) {

    if (singleton != nullptr) return singleton;

    singleton = ref new PyShell();

    return singleton;
}

