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

namespace PythonWinRT
{
	public ref class PyShell sealed
	{
            Windows::UI::Core::CoreDispatcher ^dispatcher;

            Array<unsigned char>^ simple_string;
            void RunString(const std::string& data);

            PyObject *metrosetup;
            void StartInterpreter();
            void StopInterpreter();
	public:
            PyShell();
	};
}
