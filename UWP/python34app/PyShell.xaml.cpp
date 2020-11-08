//
// PyShell.xaml.cpp
// Implementation of the PyShell.xaml class.
//

#include "pch.h"
#include "PyShell.xaml.h"
#include "Settings.xaml.h"
#include "Privacy.xaml.h"
#include "Python.h"
#include <ppltasks.h>

using namespace python34app;

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

static PyShell^ singleton;

extern "C" static PyObject *
add_to_stdout(PyObject *self, PyObject *args)
{
    Py_UNICODE *data;
    if (!PyArg_ParseTuple(args, "u", &data))
        return NULL;
    singleton->AddOutAsync(data);
    Py_RETURN_NONE;
}

extern "C" static PyObject *
add_to_stderr(PyObject *self, PyObject *args)
{
    Py_UNICODE *data;
    if (!PyArg_ParseTuple(args, "u", &data))
        return NULL;
    singleton->AddErrAsync(data);
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
    singleton->exit();
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

/* XXX crash at shutdown. Work around by keeping reference to scrollView. */
Windows::UI::Xaml::Controls::ScrollViewer^ scroll_tmp;
static wchar_t progpath[1024];
static std::wstring proghome;
PyShell::PyShell()
{
    InitializeComponent();
    history = ref new Vector<String^>();

    //SettingsPane::GetForCurrentView()->CommandsRequested += ref new TypedEventHandler<SettingsPane^, SettingsPaneCommandsRequestedEventArgs^>(this, &PyShell::get_settings_commands);

    load_settings(ApplicationData::Current, nullptr);
    /* register for changes to roaming data */
    ApplicationData::Current->DataChanged += ref new TypedEventHandler<ApplicationData^, Object^>(this, &PyShell::load_settings);

    scroll_tmp = this->scrollView;
    singleton = this;
    running = false;

    /* add metroui to builtin modules */
    PyImport_AppendInittab("metroui", PyInit_metroui);

    /* compute python path */
    Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
    Windows::Storage::StorageFolder^ installedLocation = package->InstalledLocation;
    wcscpy_s(progpath, installedLocation->Path->Data());
    
    proghome = std::wstring(progpath);

    Py_SetPythonHome((wchar_t*)proghome.c_str());

    /* XXX how to determine executable name? */
    wcscat_s(progpath, L"\\python34app.exe");
    Py_SetProgramName(progpath);
    // Continue when loaded
}

void PyShell::Loaded(Platform::Object^ sender, RoutedEventArgs^ e)
{
    RunAsync(&PyShell::StartInterpreter);
}

void PyShell::RunAsync(void (PyShell::*m)(void))
{
    if (running) {
        AddText("Error: Trying to run two async actions\n");
        return;
    }
    running = true;
    dispatcher = CoreWindow::GetForCurrentThread()->Dispatcher;
    auto t = create_task([this, m]{(this->*m)();});
    t.then([this]{
        running = false;
        auto _this = this;
        dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([_this]{
            _this->AddText(_this->prompt());
            _this->enable_input();
        }));
    });
}

/*
void PyShell::get_settings_commands(SettingsPane ^p, SettingsPaneCommandsRequestedEventArgs ^args)
{
    auto cmd = ref new SettingsCommand(L"about", L"About",
        ref new UICommandInvokedHandler([this](IUICommand ^cmd) {
            Windows::System::Launcher::LaunchUriAsync(ref new Uri(L"http://wiki.python.org/moin/MartinvonLoewis/Python%203%20For%20Metro"));
    }));
    args->Request->ApplicationCommands->Append(cmd);
    
    cmd = ref new SettingsCommand(L"privacy", L"Privacy",
        ref new UICommandInvokedHandler([this](IUICommand ^cmd) {
            auto popup = ref new Popup();
            popup->Height = this->ActualHeight;
            popup->Width = 346; // per UI guidelines
            popup->SetValue(Canvas::LeftProperty, this->ActualWidth - 346);
            popup->SetValue(Canvas::TopProperty, safe_cast<Platform::Object^>(0));
            popup->SetValue(FrameworkElement::HeightProperty, this->ActualHeight);
            popup->IsLightDismissEnabled = true;

            auto s = ref new Privacy();
            s->Height = this->ActualHeight;
            popup->Child = s;
            popup->IsOpen = true;
    }));
    args->Request->ApplicationCommands->Append(cmd);
    
    cmd = ref new SettingsCommand(L"settings", L"Settings",
        ref new UICommandInvokedHandler([this](IUICommand ^cmd) {
            auto popup = ref new Popup();
            popup->Height = this->ActualHeight;
            popup->Width = 346; // per UI guidelines
            popup->SetValue(Canvas::LeftProperty, this->ActualWidth - 346);
            popup->SetValue(Canvas::TopProperty, safe_cast<Platform::Object^>(0));
            popup->SetValue(FrameworkElement::HeightProperty, this->ActualHeight);
            popup->IsLightDismissEnabled = true;

            auto s = ref new Settings(this);
            s->Height = this->ActualHeight;
            popup->Child = s;
            popup->IsOpen = true;
    }));
    args->Request->ApplicationCommands->Append(cmd);

    cmd = ref new SettingsCommand(L"docs", L"Python Documentation",
        ref new UICommandInvokedHandler([this](IUICommand ^cmd) {
            Windows::System::Launcher::LaunchUriAsync(ref new Uri(L"http://docs.python.org/dev/index.html"));
    }));
    args->Request->ApplicationCommands->Append(cmd);

    
}
*/
void PyShell::StartInterpreter()
{
    exited = false;
    _prompt = 1;
    current_run = nullptr;
    current_paragraph = nullptr;
    current_input = nullptr;
    last_was_stderr = false;

    Py_Initialize();
    //PyEval_InitThreads();
    
    /* boot interactive shell */
    metrosetup = PyImport_ImportModule("metrosetup");
    if (metrosetup == NULL) {
        PyErr_Print();
    }

    //PyEval_ReleaseThread(PyThreadState_Get());
}

void PyShell::StopInterpreter()
{
    //PyGILState_STATE s = PyGILState_Ensure();
    Py_Finalize();
    this->textBlock1->Blocks->Clear();
}

void PyShell::OnNavigatedTo(NavigationEventArgs^ e)
{
    this->UpdateLayout();
    bool x=this->textBlock1->Focus(Windows::UI::Xaml::FocusState::Programmatic);
}

void PyShell::SizeChanged(Platform::Object^ source, Windows::UI::Xaml::SizeChangedEventArgs^ args)
{
    float width = args->NewSize.Width;
    if (current_input) {
        current_input->Width = args->NewSize.Width * 0.9;
    }
}

void PyShell::exit()
{
    exited = true;
}

String^ PyShell::prompt()
{
    switch (_prompt) {
    case 0:
        return "";
    case 1:
        return ">>> ";
    case 2:
        return "... ";
    }
    /* Error */
    return nullptr;
}

void PyShell::enable_input()
{
    current_input = ref new TextBox();
    current_input->AcceptsReturn = false;
    current_input->Width = this->ActualWidth * 0.9;
    current_input->FontFamily = ref new Windows::UI::Xaml::Media::FontFamily("Consolas");
    current_input->FontSize = 16;
    current_input->BorderBrush = ref new SolidColorBrush(forecolor);
    current_input->BorderThickness = 1;
    current_input->KeyDown += ref new KeyEventHandler(this, &PyShell::KeyDown);
    InlineUIContainer^ c = ref new InlineUIContainer();
    c->Child = current_input;
    current_paragraph->Inlines->Append(c);
    current_input->Focus(Windows::UI::Xaml::FocusState::Programmatic);
    scrollView->UpdateLayout();
    scrollView->ScrollToVerticalOffset(this->textBlock1->ActualHeight);
}

void PyShell::disable_input()
{
    if (current_input == nullptr)
        return;
    current_paragraph->Inlines->RemoveAtEnd();
    current_input = nullptr;
}

PyObject* PyShell::try_compile() 
{
    //PyGILState_STATE s = PyGILState_Ensure();
    PyObject *code = PyObject_CallMethod(metrosetup, "compile", "u", command->Data());
    if (code == NULL) {
        PyErr_Print();
        command = "";
        _prompt = 1;
    }
    else if (code == Py_None) { 
        // more input
        _prompt = 2;
        Py_DECREF(code);
    }
    else
        _prompt = 0;
    //PyGILState_Release(s);
    return code;
}

void PyShell::run_code()
{
    //PyGILState_STATE s = PyGILState_Ensure();
    PyObject *result = PyObject_CallMethod(metrosetup, "eval", "O", current_code);
    if (result == NULL) {
        PyErr_Print();
    }
    else {
        Py_DECREF(result);
    }
    Py_CLEAR(current_code);
    _prompt = 1;
    command = "";
    //PyGILState_Release(s);
    if (exited) {
        //Window::Current->Close();
        StopInterpreter();
        StartInterpreter();
    }
}

void python34app::PyShell::KeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
    if (e->Key == Windows::System::VirtualKey::Enter) {
        String^ line = current_input->Text;
        if (line != nullptr && line != "") {
            history->Append(line);
            histpos = history->Size;
        }

        disable_input();
        line = line + "\n";
        AddText(line);

        command += line;
        current_code = try_compile();
        AddText(prompt());
        if (_prompt == 0)
            RunAsync(&PyShell::run_code);
        else
            enable_input();
        e->Handled = true;
    }

    /*
    if (e->Key == Windows::System::VirtualKey::Tab) {
        textBox1->
        e->Handled = true;
    }
    */

    if (e->Key == Windows::System::VirtualKey::Up && histpos > 0) {
        histpos--;
        current_input->Text = history->GetAt(histpos);
        e->Handled = true;
    }

    if (e->Key == Windows::System::VirtualKey::Down && histpos < history->Size) {
        histpos++;
        if (histpos < history->Size) {
            current_input->Text = history->GetAt(histpos);
        }
        e->Handled = true;
    }
}

void PyShell::AddText(String ^s0, bool is_stderr)
{
    const wchar_t *s = s0->Data();
            
    if (last_was_stderr != is_stderr) {
        current_run = nullptr;
    }
    last_was_stderr = is_stderr;
    if (current_run == nullptr) {
        current_paragraph = ref new Paragraph();
        current_paragraph->FontFamily = ref new Windows::UI::Xaml::Media::FontFamily("Consolas");
        current_paragraph->FontSize = 16;
        if (is_stderr) {
            current_paragraph->Foreground = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Red);
        }
        current_run = ref new Run();
        current_paragraph->Inlines->Append(current_run);
        this->textBlock1->Blocks->Append(current_paragraph);
    }
    while(*s) {
        if (*s == L'\n') {
            current_run = nullptr;
            AddText(ref new String(s+1), is_stderr);
            return;
        }
        if (current_run->Text == nullptr)
            current_run->Text = ref new String(s, 1);
        else
            current_run->Text += (wchar_t)*s;
        s++;
    }
    scrollView->UpdateLayout();
    scrollView->ScrollToVerticalOffset(this->textBlock1->ActualHeight);
}

void PyShell::AddTextAsync(String ^s0, bool is_stderr)
{
    dispatcher->RunAsync(CoreDispatcherPriority::Normal,
        ref new DispatchedHandler([this,s0,is_stderr]{
            AddText(s0, is_stderr);
    }));
}

void PyShell::AddOutAsync(wchar_t *s)
{
    AddTextAsync(ref new String(s), false);
}

void PyShell::AddErrAsync(wchar_t *s)
{
    AddTextAsync(ref new String(s), true);
}

void python34app::PyShell::do_restart(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    StopInterpreter();
    StartInterpreter();
}

void PyShell::run_simple_string()
{
    //PyGILState_STATE s = PyGILState_Ensure();
    PyRun_SimpleString((char*)simple_string->Data);
    //PyGILState_Release(s);
}

void python34app::PyShell::run_file(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    FileOpenPicker^ fop = ref new FileOpenPicker();
    fop->SuggestedStartLocation = PickerLocationId::DocumentsLibrary;
    fop->CommitButtonText = "Run Script";
    fop->FileTypeFilter->Append(".py");
    auto t = create_task(fop->PickSingleFileAsync());
    t.then([this](StorageFile^ file){
        if (file == nullptr) {
            AddErrAsync(L"Cancelled");
        }
        else {
            auto _this = this;
            auto open = create_task(file->OpenReadAsync());
            open.then([_this, file](IRandomAccessStream^ f) {
                if (f == nullptr) {
                    wchar_t msg[1024];
                    swprintf_s(msg, L"Reading %s failed", file->Path->Data());
                    _this->AddErrAsync(msg);
                } else {
                    auto reader = ref new DataReader(f);
                    auto reading = create_task(reader->LoadAsync(f->Size));
                    reading.then([_this, reader](UINT bytesread){
                        _this->simple_string = ref new Array<unsigned char>(bytesread);
                        reader->ReadBytes(_this->simple_string);
                        _this->simple_string->Data[bytesread] = '\0';
                        _this->RunAsync(&PyShell::run_simple_string);
                    }) ;
                }
            });
        }
    });
}


/***************************** Settings *********************/

void PyShell::set_forecolor(Color c)
{
    forecolor = c;
    auto brush = ref new SolidColorBrush(c);
    textBlock1->Foreground = brush;
    if (current_input) {
        /* Cannot set background since caret color will stay in black */
        current_input->BorderBrush = brush;
    }
}

void PyShell::set_backcolor(Color c)
{
    backcolor = c;
    auto brush = ref new SolidColorBrush(c);
    scrollView->Background = brush;
}

static uint32_t color2int(Color c)
{
    return (((((c.A << 8) + c.R) << 8) + c.G) << 8) + c.B;
}

static Color int2color(uint32_t c)
{
    uint8_t a,r,g,b;
    b = c & 0xff; c >>= 8;
    g = c & 0xff; c >>= 8;
    r = c & 0xff; c >>= 8;
    a = c & 0xff; c >>= 8;
    return ColorHelper::FromArgb(a, r, g, b);
}

void PyShell::load_settings(ApplicationData^ data, Object^)
{
    auto values = data->RoamingSettings->Values;
    auto fg = values->Lookup(L"forecolor");
    if (fg != nullptr)
        set_forecolor(int2color(safe_cast<uint32_t>(fg)));
    auto bg = values->Lookup(L"backcolor");
    if (bg != nullptr)
        set_backcolor(int2color(safe_cast<uint32_t>(bg)));
}

void PyShell::save_settings()
{
    auto values = ApplicationData::Current->RoamingSettings->Values;
    values->Insert(L"forecolor", color2int(forecolor));
    values->Insert(L"backcolor", color2int(backcolor));
}