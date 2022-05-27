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
using System.Numerics;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Drawing;
using System.Drawing.Imaging;
using System.Security;
using Image = System.Drawing.Image;
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
        public string DefaultOfdPath = "D:\\Deze PC\\Documenten\\GitHub\\Project_TBD\\TestFiles"; // c:\\
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
            var timeout = new TimeSpan(0, 0, 0, 5, 0);
            var message = string.Empty;
            string returnMessage;

            var push = new PushSocket();
            var sub = new SubscriberSocket();

            push.Connect(ServerPush);
            sub.Connect(ServerSub);

            Print("Requesting UUID");
            var uuid = string.Empty;

            returnMessage = string.Empty;

            message = "LennyIndustries|LIES_Server|UUID";

            sub.Subscribe("LennyIndustries|LIES_UUID|");
            push.SendFrame(message);
            sub.TryReceiveFrameString(timeout, out returnMessage);

            if (returnMessage == string.Empty)
            {
                Print("Did not receive UUID");
                MessageBox.Show("Did not receive UUID.", "Send error", MessageBoxButton.OK, MessageBoxImage.Asterisk);
                return;
            }
            uuid = returnMessage.Substring(26);

            //Print(uuid);

            Print("Generating key");
            var csp = new RSACryptoServiceProvider(2048);
            TextWriter publicKeyString = new StringWriter();

            var privateKey = csp.ExportParameters(true);

            ExportPublicKey(csp, publicKeyString);
            //Print(publicKeyString.ToString());

            Print("Requesting key");
            var key = string.Empty;

            returnMessage = string.Empty;

            message = "LennyIndustries|LIES_Server|Key";

            sub.Subscribe("LennyIndustries|LIES_Key|");
            push.SendFrame(message);
            sub.TryReceiveFrameString(timeout, out returnMessage);

            if (returnMessage == string.Empty)
            {
                Print("Did not receive key");
                MessageBox.Show("Did not receive key.", "Send error", MessageBoxButton.OK, MessageBoxImage.Asterisk);
                return;
            }
            key = returnMessage.Substring(25);

            //Print(key);

            Print("Collecting data");
            var image = ImageTextBox.Text;
            var text = TextTextBox.Text;
            var mask = 0x1;
            var checkOptions = 0;
            var check = 0;

            csp = new RSACryptoServiceProvider();
            csp.ImportParameters(privateKey);

            message = "LennyIndustries|LIES_Server|";
            message += Encrypt ? "Encrypt|" : "Decrypt|";
            message += "UUID=" + uuid + ":";
            message += "Key=" + publicKeyString + ":";

            checkOptions = GetOption(out _);

            check = mask & checkOptions;
            if (check != 0)
            {
                var password = PasswordBox.Password;
                message += "Password=" + password + ":"; // Convert.ToBase64String(csp.Encrypt(Encoding.Unicode.GetBytes(X), false))
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
                /**
                FileStream fs = File.OpenRead(image);
                byte[] b = new byte[fs.Length];
                UTF8Encoding temp = new UTF8Encoding(true);
                string imageToSend = string.Empty;
                while (fs.Read(b, 0, b.Length) > 0)
                {
                    imageToSend += temp.GetString(b);
                }
                **/
                /**
                var fs = new FileStream(image, FileMode.Open);
                var fileData = new byte[fs.Length];
                fs.Read(fileData, 0, fileData.Length);
                fs.Close();
                **/
                /**
                string line;
                var imageReader = new StreamReader(image, Encoding.Default, true);
                var imageList = new List<char>();
                Console.WriteLine(@"Testing image");
                while (imageReader.Peek() > -1)
                {
                    line = imageReader.ReadLine();
                    foreach (char c in line)
                    {
                        Console.WriteLine(c);
                        imageList.Add(c);
                    }
                }

                var imageArray = imageList.ToArray();
                **/
                /**
                MemoryStream ms = new MemoryStream();
                Bitmap bm = new Bitmap(image);
                bm.Save(ms, ImageFormat.Jpeg);
                var imageString = Convert.ToBase64String(ms.ToArray());
                StreamWriter sw = new StreamWriter(@"D:\Deze PC\Documenten\GitHub\Project_TBD\cmake-build-debug\temp.txt", false);
                sw.Write(imageString);
                sw.Close();
                **/

                Bitmap bm = (Bitmap)Image.FromFile(image);
                string bmString = bm.ToString();

                //string imageToSend = new string(Encoding.ASCII.GetChars(fileData));

                message += "ImageLength=" + bmString.Length + ":Image=" + bmString; // System.Text.Encoding.UTF8.GetString(fileData)

                //Print(System.Text.Encoding.Default.GetString(fileData));
            }

            //Print(message);

            Print("Encrypting data");




            Print("Sending data");
            var output = OutputTextBox.Text;

            returnMessage = string.Empty;
            message = message.Remove(message.Length - 1);

            sub.Subscribe("LennyIndustries|LIES_Client|" + uuid + "|");
            push.SendFrame(message);
            sub.TryReceiveFrameString(timeout, out var msg);
        }

        private static void ExportPublicKey(RSACryptoServiceProvider csp, TextWriter outputStream)
        {
            var parameters = csp.ExportParameters(false);
            using (var stream = new MemoryStream())
            {
                var writer = new BinaryWriter(stream);
                writer.Write((byte)0x30); // SEQUENCE
                using (var innerStream = new MemoryStream())
                {
                    var innerWriter = new BinaryWriter(innerStream);
                    innerWriter.Write((byte)0x30); // SEQUENCE
                    EncodeLength(innerWriter, 13);
                    innerWriter.Write((byte)0x06); // OBJECT IDENTIFIER
                    var rsaEncryptionOid = new byte[] { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01 };
                    EncodeLength(innerWriter, rsaEncryptionOid.Length);
                    innerWriter.Write(rsaEncryptionOid);
                    innerWriter.Write((byte)0x05); // NULL
                    EncodeLength(innerWriter, 0);
                    innerWriter.Write((byte)0x03); // BIT STRING
                    using (var bitStringStream = new MemoryStream())
                    {
                        var bitStringWriter = new BinaryWriter(bitStringStream);
                        bitStringWriter.Write((byte)0x00); // # of unused bits
                        bitStringWriter.Write((byte)0x30); // SEQUENCE
                        using (var paramsStream = new MemoryStream())
                        {
                            var paramsWriter = new BinaryWriter(paramsStream);
                            EncodeIntegerBigEndian(paramsWriter, parameters.Modulus); // Modulus
                            EncodeIntegerBigEndian(paramsWriter, parameters.Exponent); // Exponent
                            var paramsLength = (int)paramsStream.Length;
                            EncodeLength(bitStringWriter, paramsLength);
                            bitStringWriter.Write(paramsStream.GetBuffer(), 0, paramsLength);
                        }
                        var bitStringLength = (int)bitStringStream.Length;
                        EncodeLength(innerWriter, bitStringLength);
                        innerWriter.Write(bitStringStream.GetBuffer(), 0, bitStringLength);
                    }
                    var length = (int)innerStream.Length;
                    EncodeLength(writer, length);
                    writer.Write(innerStream.GetBuffer(), 0, length);
                }

                var base64 = Convert.ToBase64String(stream.GetBuffer(), 0, (int)stream.Length).ToCharArray();
                outputStream.WriteLine("-----BEGIN PUBLIC KEY-----");
                for (var i = 0; i < base64.Length; i += 64)
                {
                    outputStream.WriteLine(base64, i, Math.Min(64, base64.Length - i));
                }
                outputStream.WriteLine("-----END PUBLIC KEY-----");
            }
        }

        private static void EncodeLength(BinaryWriter stream, int length)
        {
            if (length < 0) throw new ArgumentOutOfRangeException("length", "Length must be non-negative");
            if (length < 0x80)
            {
                // Short form
                stream.Write((byte)length);
            }
            else
            {
                // Long form
                var temp = length;
                var bytesRequired = 0;
                while (temp > 0)
                {
                    temp >>= 8;
                    bytesRequired++;
                }
                stream.Write((byte)(bytesRequired | 0x80));
                for (var i = bytesRequired - 1; i >= 0; i--)
                {
                    stream.Write((byte)(length >> (8 * i) & 0xff));
                }
            }
        }

        private static void EncodeIntegerBigEndian(BinaryWriter stream, byte[] value, bool forceUnsigned = true)
        {
            stream.Write((byte)0x02); // INTEGER
            var prefixZeros = 0;
            for (var i = 0; i < value.Length; i++)
            {
                if (value[i] != 0) break;
                prefixZeros++;
            }
            if (value.Length - prefixZeros == 0)
            {
                EncodeLength(stream, 1);
                stream.Write((byte)0);
            }
            else
            {
                if (forceUnsigned && value[prefixZeros] > 0x7f)
                {
                    // Add a prefix zero to force unsigned if the MSB is 1
                    EncodeLength(stream, value.Length - prefixZeros + 1);
                    stream.Write((byte)0);
                }
                else
                {
                    EncodeLength(stream, value.Length - prefixZeros);
                }
                for (var i = prefixZeros; i < value.Length; i++)
                {
                    stream.Write(value[i]);
                }
            }
        }

        private void Print(string msg)
        {
            ProgramOutput.Text = ProgramOutput.Text + Environment.NewLine + msg;
            ProgramOutput.ScrollToEnd();
        }

        private bool CheckServer(string connectionOption)
        {
            Print("Pinging server");
            var timeout = new TimeSpan(0, 0, 0, 1, 0);
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
            try
            {
                DefaultOfdPath = Path.GetDirectoryName(ofd.FileName);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                MessageBox.Show(e.Message, "ERROR", MessageBoxButton.OK, MessageBoxImage.Error);
                return "Path Invalid";
            }
            return ofd.FileName;
        }

        private void TestConnectionBtn_OnClick(object sender, RoutedEventArgs e)
        {
            Mouse.OverrideCursor = System.Windows.Input.Cursors.Wait;
            UpdateConnection();
            Mouse.OverrideCursor = null;
        }

        private void SendBtn_OnClick(object sender, RoutedEventArgs e)
        {
            Mouse.OverrideCursor = System.Windows.Input.Cursors.Wait;
            UpdateConnection();
            ValidateInput();

            if (ServerIsOnline && InputsAreValid)
            {
                Handle();
            }
            Mouse.OverrideCursor = null;
        }

        private void OptionsBtn_OnClick(object sender, RoutedEventArgs e)
        {
            Mouse.OverrideCursor = System.Windows.Input.Cursors.Wait;
            ValidateInput();
            Mouse.OverrideCursor = null;
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
