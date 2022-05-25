using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.ServiceModel;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;
using NetMQ;
using NetMQ.Sockets;
using System.IO;
using Path = System.IO.Path;

namespace LIES_Client
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>

    public partial class MainWindow : Window
    {
        public bool ServerIsOnline = false;
        public bool InputsAreValid = false;
        public string ServerPush = string.Empty;
        public string ServerSub = string.Empty;
        public string DefaultOfdPath = "c:\\";
        public bool Encrypt;
        public MainWindow()
        {
            InitializeComponent();

            ConnectionCb.Items.Add("Loopback");
            ConnectionCb.Items.Add("Local");
            ConnectionCb.Items.Add("Internet");
            ConnectionCb.SelectedItem = "Internet";
            ServerStatus.Fill = new SolidColorBrush(Colors.Orange);

            OptionsCb.Items.Add("Encrypt text in image with extra encryption on text");
            OptionsCb.Items.Add("Encrypt text in image with plain text");
            OptionsCb.Items.Add("Decrypt text from image with extra decryption on text");
            OptionsCb.Items.Add("Decrypt text from image with plain text");
            OptionsCb.Items.Add("Encrypt text only with password");
            OptionsCb.Items.Add("Decrypt text only with password");
            OptionsCb.Items.Add("CRC32 of text");
            OptionsCb.Items.Add("CRC32 of image");
            OptionsCb.SelectedItem = "Encrypt text in image with extra encryption on text";
            OptionStatus.Fill = new SolidColorBrush(Colors.Orange);

            ProgramOutput.VerticalScrollBarVisibility = ScrollBarVisibility.Visible;
            ProgramOutput.IsReadOnly = true;

            ImageTextBox.IsReadOnly = true;
            TextTextBox.IsReadOnly = true;
            OutputTextBox.IsReadOnly = true;
        }

        private void Handle()
        {
            Print("Sending data");
            var selectedOption = OptionsCb.SelectedIndex;
            var image = ImageTextBox.Text;
            var text = TextTextBox.Text;
            var output = OutputTextBox.Text;
            var mask = 0x1;
            var checkOptions = 0;
            var check = 0;
            var uuid = string.Empty;

            var timeout = new TimeSpan(0, 0, 0, 5, 0);
            var message = "LennyIndustries|LIES_Server|";

            message += Encrypt ? "Encrypt|" : "Decrypt|";

            var push = new PushSocket();
            var sub = new SubscriberSocket();

            push.Connect(ServerPush);
            sub.Connect(ServerSub);

            checkOptions = GetOption(out _);

            check = mask & checkOptions;
            if (check != 0)
            {
                var password = PasswordBox.Password;
                message += "Password=" + password + ":";
            }

            mask <<= 2;
            check = mask & checkOptions;
            if (check != 0)
            {
                var textData = File.ReadAllText(text);
                message += "TextLength=" + textData.Length + ":Text=" + textData + ":";
            }

            mask <<= 2;
            check = mask & checkOptions;
            if (check != 0)
            {
                var fs = new FileStream(image, FileMode.Open);
                var fileData = new byte[fs.Length];
                fs.Read(fileData, 0, fileData.Length);
                fs.Close();
                var imageLength = fileData.Length;

                message += "ImageLength=" + imageLength + ":Image=" + System.Text.Encoding.UTF8.GetString(fileData) + ":";
            }

            sub.Subscribe("LennyIndustries|LIES_Client|" + uuid);
            push.SendFrame(message);
            sub.TryReceiveFrameString(timeout, out var msg);
        }

        private void Print(string msg)
        {
            ProgramOutput.Text = ProgramOutput.Text + Environment.NewLine + msg;
            ProgramOutput.ScrollToEnd();
        }

        private bool CheckServer(string connectionOption)
        {
            Print("Pinging server");
            var timeout = new TimeSpan(0, 0, 0, 0, 500);
            var connectTo = "tcp://";
            var receiveFrom = "tcp://";

            switch (connectionOption)
            {
                case "Loopback":
                    connectTo += "localhost";
                    receiveFrom += "localhost";
                    break;
                case "Local":
                    connectTo += "192.168.1.8";
                    receiveFrom += "192.168.1.8";
                    break;
                case "Internet":
                    connectTo += "193.190.154.184";
                    receiveFrom += "193.190.154.184";
                    break;
                default:
                    ServerStatus.Fill = new SolidColorBrush(Colors.Orange);
                    MessageBox.Show("Please select a connection option.", "No connection option", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    return false;
            }
            connectTo += ":24041";
            receiveFrom += ":24042";

            ServerPush = connectTo;
            ServerSub = receiveFrom;

            var push = new PushSocket();
            var sub = new SubscriberSocket();

            push.Connect(connectTo);
            sub.Connect(receiveFrom);

            sub.Subscribe("LennyIndustries|LIES_Pong");
            push.SendFrame("LennyIndustries|LIES_Server|Ping");
            sub.TryReceiveFrameString(timeout, out var msg);

            return (msg == "LennyIndustries|LIES_Pong");
        }

        private void UpdateConnection()
        {
            if (CheckServer(ConnectionCb.Text))
            {
                ServerStatus.Fill = new SolidColorBrush(Colors.Green);
                ServerIsOnline = true;
            }
            else
            {
                ServerStatus.Fill = new SolidColorBrush(Colors.Red);
                ServerIsOnline = false;
                MessageBox.Show("The server is not responding. Please try again.", "Server offline", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private int GetOption(out string extension)
        {
            var selectedOption = OptionsCb.SelectedIndex;
            switch (selectedOption)
            {
                case 0:
                    Print("Option: Encrypt text in image with extra encryption on text");
                    extension = ".bmp";
                    Encrypt = true;
                    return 63;
                case 1:
                    Print("Option: Encrypt text in image with plain text");
                    extension = ".bmp";
                    Encrypt = true;
                    return 62;
                case 2:
                    Print("Option: Decrypt text from image with extra decryption on text");
                    extension = ".txt";
                    Encrypt = false;
                    return 51;
                case 3:
                    Print("Option: Decrypt text from image with plain text");
                    extension = ".txt";
                    Encrypt = false;
                    return 50;
                case 4:
                    Print("Option: Encrypt text only with password");
                    extension = ".txt";
                    Encrypt = true;
                    return 15;
                case 5:
                    Print("Option: Decrypt text only with password");
                    extension = ".txt";
                    Encrypt = false;
                    return 15;
                case 6:
                    Print("Option: CRC32 of text");
                    extension = ".txt";
                    Encrypt = false;
                    return 14;
                case 7:
                    Print("Option: CRC32 of image");
                    extension = ".txt";
                    Encrypt = false;
                    return 50;
            }
            extension = string.Empty;
            return 0;
        }

        private void ValidateInput()
        {
            Print("Validating input");
            var image = ImageTextBox.Text;
            var text = TextTextBox.Text;
            var output = OutputTextBox.Text;
            var mask = 0x1;
            var checkOptions = 0;
            var check = 0;
            var extension = string.Empty;
            var redBrush = new SolidColorBrush(Colors.Red);
            var whiteBrush = new SolidColorBrush(Colors.White);

            ImageTextBox.Background = whiteBrush;
            TextTextBox.Background = whiteBrush;
            OutputTextBox.Background = whiteBrush;
            PasswordBox.Background = whiteBrush;

            InputsAreValid = true;

            checkOptions = GetOption(out extension);

            check = mask & checkOptions;
            if (check != 0)
            {
                if (PasswordBox.Password == string.Empty)
                {
                    InputsAreValid = false;
                    PasswordBox.Background = redBrush;
                    Print("No password is given");
                }
            }

            mask <<= 1;
            check = mask & checkOptions;
            if (check != 0)
            {
                if (Path.GetExtension(output) != extension)
                {
                    InputsAreValid = false;
                    OutputTextBox.Background = redBrush;
                    Print("Output has invalid extension");
                }
            }

            mask <<= 1;
            check = mask & checkOptions;
            if (check != 0)
            {
                if (!File.Exists(text))
                {
                    InputsAreValid = false;
                    TextTextBox.Background = redBrush;
                    Print("Text does not exist");
                }
            }

            mask <<= 1;
            check = mask & checkOptions;
            if (check != 0)
            {
                if (Path.GetExtension(text) != ".txt")
                {
                    InputsAreValid = false;
                    TextTextBox.Background = redBrush;
                    Print("Text has invalid extension");
                }
            }

            mask <<= 1;
            check = mask & checkOptions;
            if (check != 0)
            {
                if (!File.Exists(image))
                {
                    InputsAreValid = false;
                    ImageTextBox.Background = redBrush;
                    Print("Image does not exist");
                }
            }

            mask <<= 1;
            check = mask & checkOptions;
            if (check != 0)
            {
                if (Path.GetExtension(image) != ".bmp")
                {
                    InputsAreValid = false;
                    ImageTextBox.Background = redBrush;
                    Print("Image has invalid extension");
                }
            }

            if (InputsAreValid)
            {
                OptionStatus.Fill = new SolidColorBrush(Colors.Green);
                Print("Inputs are valid");
                return;
            }

            OptionStatus.Fill = new SolidColorBrush(Colors.Red);
            Print("Inputs are invalid");
        }

        private string FileDialog(string filter, bool exist = true)
        {
            Print("Opening file dialog");
            var ofd = new OpenFileDialog
            {
                InitialDirectory = DefaultOfdPath,
                Filter = filter,
                FilterIndex = 1,
                RestoreDirectory = true,
                Multiselect = false,
                CheckFileExists = exist
            };

            ofd.ShowDialog();
            DefaultOfdPath = Path.GetDirectoryName(ofd.FileName);
            return ofd.FileName;
        }

        private void TestConnectionBtn_OnClick(object sender, RoutedEventArgs e)
        {
            UpdateConnection();
        }

        private void SendBtn_OnClick(object sender, RoutedEventArgs e)
        {
            UpdateConnection();
            ValidateInput();

            if (ServerIsOnline && InputsAreValid)
            {
                Handle();
            }
        }

        private void OptionsBtn_OnClick(object sender, RoutedEventArgs e)
        {
            ValidateInput();
        }

        private void ImageBtn_OnClick(object sender, RoutedEventArgs e)
        {
            ImageTextBox.Text = FileDialog("Bitmap image (*.bmp)|*.bmp|All files (*.*)|*.*");
        }

        private void TextBtn_OnClick(object sender, RoutedEventArgs e)
        {
            TextTextBox.Text = FileDialog("Text files (*.txt)|*.txt|All files (*.*)|*.*");
        }

        private void OutputBtn_OnClick(object sender, RoutedEventArgs e)
        {
            OutputTextBox.Text = FileDialog("All files (*.*)|*.*|Text files (*.txt)|*.txt|Bitmap image (*.bmp)|*.bmp", false);
        }
    }
}
