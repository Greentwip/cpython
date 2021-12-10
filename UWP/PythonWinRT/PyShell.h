//
// PyShell.xaml.h
// Declaration of the PyShell.xaml class.
//

#pragma once

#include "pch.h"
#include "Python.h"
using namespace Platform;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::ApplicationSettings;



using namespace Windows::Storage::Streams;

namespace PythonWinRT
{
    public delegate void OutputHandler(String^ s);

   

	public ref class PyShell sealed
	{
            Windows::UI::Core::CoreDispatcher ^dispatcher;

            Array<unsigned char>^ simple_string;
            void RunString(const std::string& data);

            PyObject *metrosetup;

    internal:
            void PyShell::addOut(Py_UNICODE *m);
            void PyShell::addError(Py_UNICODE *m);
            void PyShell::addDebug(Py_UNICODE *m);
            void PyShell::echoOut(Py_UNICODE *m);
            void PyShell::echoOut(String^ m);

            

            PyShell();
            
	public:
            
           
            void StartInterpreter();
            void StopInterpreter();
            void RunSimpleString(Platform::String^ data);
            void PyShell::ImportModule(String^ pmoduleName);
            /// <summary>
            /// Executes a function that receives a string and returns a string
            /// </summary>
            /// <param name="pmoduleName">Module where to find the function</param>
            /// <param name="pfuncName">name of the function that receives a string and returns a string</param>
            /// <param name="pparam">string parameter for the function</param>
            /// <returns>string result</returns>
            String^ PyShell::ExecuteStringFunc(String^ pmoduleName, String^ pfuncName, String^ pparam);
           
            
           
            //Events
            event OutputHandler^ ConsoleOutputEvent;
            event OutputHandler^ ErrorOutputEvent;
            event OutputHandler^ DebugOutputEvent;
            event OutputHandler^ EchoOutputEvent;

	};


    public ref class RTPython sealed
    {


    public:
        /// <summary>
        /// The shell is a singleton, this will always return the same instance.
        /// </summary>
        /// <param name=""></param>
        /// <returns></returns>
        PyShell^ RTPython::GetShell(void);

    };
}
