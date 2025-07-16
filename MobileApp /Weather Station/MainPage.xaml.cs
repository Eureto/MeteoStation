using ProjektDotNet;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Net;
using System.Threading.Tasks;
using Android.Net.Wifi;

#if ANDROID
using Android.Content;
using Android.Net;
using Android;
#endif 
namespace Weather_Station
{
    public partial class MainPage : ContentPage
    {
        int count = 0;

        public MainPage()
        {
            InitializeComponent();
        }

        private async void buttonConnect_Click(object sender, EventArgs e)
        {

            string deviceAddress1 = textBoxDeviceAddress1.Text;
            string deviceAddress2 = textBoxDeviceAddress2.Text;
            string deviceAddress3 = textBoxDeviceAddress3.Text;
            string deviceAddress4 = textBoxDeviceAddress4.Text;
            string deviceAddress = deviceAddress1 + "." + deviceAddress2 + "." + deviceAddress3 + "." + deviceAddress4;
            //sprawdź czy adres ip jest poprawny i mieści się w zakresie 0-255
            try
            {
                // Jeśli wpisany adres jest pusty albo niepoprawne wartośći to wywala komunikat i resetuje textBoxy
                if (deviceAddress1 == "" || deviceAddress2 == "" || deviceAddress3 == "" || deviceAddress4 == ""
                || int.Parse(deviceAddress1) > 255 || int.Parse(deviceAddress2) > 255 || int.Parse(deviceAddress3) > 255 || int.Parse(deviceAddress4) > 255 ||
                int.Parse(deviceAddress1) < 0 || int.Parse(deviceAddress2) < 0 || int.Parse(deviceAddress3) < 0 || int.Parse(deviceAddress4) < 0)
                {
                    await DisplayAlert("Błąd", "Podaj poprawny adres IP urządzenia", "OK");
                    textBoxDeviceAddress1.Text = "";
                    textBoxDeviceAddress2.Text = "";
                    textBoxDeviceAddress3.Text = "";
                    textBoxDeviceAddress4.Text = "";
                    return;
                }
            }
            catch
            {
                await DisplayAlert("Błąd", "Podaj poprawny adres IP urządzenia", "OK");
                textBoxDeviceAddress1.Text = "";
                textBoxDeviceAddress2.Text = "";
                textBoxDeviceAddress3.Text = "";
                textBoxDeviceAddress4.Text = "";
                return;
            }
            deviceAddress = "http://" + deviceAddress;
            labelConnecting.IsVisible = true;
            try
            {
                WeatherParam wp = WeatherParam.GetValuesFromDevice(deviceAddress);
                if (wp == null)
                {
                    await DisplayAlert("Błąd", "Nie udało się połączyć z urządzeniem, wybierz inne urządzenie", "OK");
                    labelConnecting.IsVisible = false;
                    return;
                }
                //MessageBox.Show("Time: " + wp.date + "\nTemperature: " + wp.temperature + "°C\nHumidity: " + wp.humidity + "%\nPressure: " + wp.pressure + "hPa");
            }
            catch
            {
                await DisplayAlert("Błąd", "Nie udało się połączyć z urządzeniem, wybierz inne urządzenie", "OK");
                labelConnecting.IsVisible = false;
                return;
            }


            //zapisz konfigurację do pliku gdzie jest exe
            string path = AppDomain.CurrentDomain.BaseDirectory + "config.txt";
            string[] lines = { deviceAddress };
            try
            {
                System.IO.File.WriteAllLines(path, lines);
            }
            catch (Exception ex)
            {
                await DisplayAlert("Błąd", "Nie udało się zapisać konfiguracji do pliku: " + ex.Message, "OK");
            }

            App.isConfig = true;
            App.hasAddress = true;
            //Open new window

            ////przekaż adres ip do nowego okna
            await Navigation.PushAsync(new ShowData(true));
            ShowData.deviceAddress = deviceAddress;
            labelConnecting.IsVisible = false;

        }

        private async Task PingDeviceAsync(string deviceAddress, List<string> listOfIpAddresses)
        {
            using (Ping pinger = new Ping())
            {
                try
                {
                    //Wysyłam asynchoroniczne zapytanie ping do urządzenia 
                    PingReply reply = await pinger.SendPingAsync(deviceAddress, 100);
                    if (reply.Status == IPStatus.Success)
                    {
                        // lock - upewnia się że tylko jeden wątek na raz modyfikuje listę
                        lock (listOfIpAddresses)
                        {
                            listOfIpAddresses.Add(deviceAddress);
                        }
                        Console.WriteLine("Adres znalezionego urządzenia: " + deviceAddress);
                    }//jeśli nie udało się połączyć z urządzeniem to czekam 100ms i ponownie próbuje połączyć się z urządzeniem
                    else
                    {
                        await Task.Delay(200);
                        PingReply reply2 = await pinger.SendPingAsync(deviceAddress, 100);
                        if (reply2.Status == IPStatus.Success)
                        {
                            lock (listOfIpAddresses)
                            {
                                listOfIpAddresses.Add(deviceAddress);
                            }
                            Console.WriteLine("Adres znalezionego urządzenia: " + deviceAddress);
                        }
                    }
                }
                catch (PingException)
                {

                }
            }
        }

        private async void buttonScanForDevice_Click(object sender, EventArgs e)
        {
            string myip = null;
            //if platform is android then ask for permission
            #if ANDROID
                WifiManager wifiManager = (WifiManager)Android.App.Application.Context.GetSystemService(Context.WifiService);
                int ipaddress = wifiManager.ConnectionInfo.IpAddress;
                IPAddress ipAddr = new IPAddress(ipaddress);
                myip = ipAddr.ToString();
            #endif

            #if WINDOWS
                //uzyskanie adresu ip urządzenia
                var host = Dns.GetHostEntry(Dns.GetHostName());

                foreach (var ip in host.AddressList)
                {
                    if (ip.AddressFamily == AddressFamily.InterNetwork)
                    {
                        myip = ip.ToString();
                        break;
                    }
                }
            #endif
            
            List<string> listOfIpAddresses = new List<string>();
            if (myip != null)
            {
                string[] myIPSplit = myip.Split('.');
                string baseIp = myIPSplit[0] + "." + myIPSplit[1] + "." + myIPSplit[2] + ".";
                List<Task> pingTasks = new List<Task>();

                //Wywołuje asynchroniczne zapytania ping do wszystkich urządzeń w sieci lokalnej
                //Zakładam że urządznie jest w sieci o masce /24 i skanuje tylko ostatni oktet adresu ip
                for (int i = 1; i < 255; i++)
                {
                    string deviceAddress = baseIp + i;
                    pingTasks.Add(PingDeviceAsync(deviceAddress, listOfIpAddresses));
                }

                await Task.WhenAll(pingTasks);

                //sortuję listę adresów ip
                listOfIpAddresses.Sort();
                //wyświetlam adresy ip w listBox
                ListViewLocalAdresses.ItemsSource = null;
                ListViewLocalAdresses.ItemsSource = listOfIpAddresses;


            }
        }

        private void ListViewLocalAddresses_SelectedIndexChanged(object sender, EventArgs e)
        {
            //zamień zaznaczony adres ip w textBoxDeviceAddress
            string selectedIp = ListViewLocalAdresses.SelectedItem.ToString();
            string[] selectedIpSplit = selectedIp.Split('.');
            textBoxDeviceAddress1.Text = selectedIpSplit[0];
            textBoxDeviceAddress2.Text = selectedIpSplit[1];
            textBoxDeviceAddress3.Text = selectedIpSplit[2];
            textBoxDeviceAddress4.Text = selectedIpSplit[3];
        }
    }

}
