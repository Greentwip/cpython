using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Documents;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace PythonSharpInterpreter
{
    
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        PythonWinRT.PyShell shell;
        public MainPage()
        {
            this.InitializeComponent();


            shell = new PythonWinRT.RTPython().GetShell();
            shell.ConsoleOutputEvent += Shell_ConsoleOutputEvent;
            shell.ErrorOutputEvent += Shell_ErrorOutputEvent;
            shell.EchoOutputEvent += Shell_EchoOutputEvent;
            shell.StartInterpreter();
           
                        
            

           // var result1 = ps.ExecuteStringFunc("simplestreamlink", "printData", "http://httpbin.org/robots.txt");

            //var result2 = ps.ExecuteStringFunc("simplestreamlink", "printData2", "http://httpbin.org/robots.txt");
           
          
        }

        private async void Shell_EchoOutputEvent(string s)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {

                var line = new Run() { Text = s, Foreground = new SolidColorBrush(Colors.Blue) };


                this.outTextBox.Inlines.Add(line);

                this.outTextBox.Inlines.Add(new LineBreak());
                this.outTextBox.Inlines.Add(new LineBreak());



            });
        }

        private void ExecuteString() {

            this.outTextBox.Text = "";                           

            shell.RunSimpleString(inputTextBox.Text);

        }

        private async void Shell_ErrorOutputEvent(string s)
        {

            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {

                var line = new Run() { Text = s, Foreground = new SolidColorBrush(Colors.Red) };
                              

                this.outTextBox.Inlines.Add(line);



            });

          

           
        }

        private async void Shell_ConsoleOutputEvent(string s)
        {

            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                var line = new Run();

                line.Text = s;
                this.outTextBox.Inlines.Add(line);
            });
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            ExecuteString();
        }
    }
}
