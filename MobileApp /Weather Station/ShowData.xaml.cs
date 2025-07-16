using ProjektDotNet;

namespace Weather_Station;

public partial class ShowData : ContentPage
{
    public static string deviceAddress;
    private List<WeatherParam> paramList;
    private bool stopExecuting = false;
    
    private async Task update()
    {
        while (!stopExecuting)
        {
            await UpdateValuesAsync();
            await Task.Delay(1000); // czekaj jedn¹ sekundê
        }
    }
    private void DownloadHistoryButton(object sender, EventArgs e)
    {
        string historyData = WeatherParam.DownloadHistoryData(deviceAddress);
        if (historyData != null)
        {
            List<string> historyList = historyData.Split('\n').ToList();
            ListViewHistoryData.ItemsSource = null;
            ListViewHistoryData.ItemsSource = historyList;
        }
        else
        {
            // Handle the case where historyData is null

        
        }
    }
    private async Task UpdateValuesAsync()
    {
        try
        {
            WeatherParam wp = await Task.Run(() => WeatherParam.GetValuesFromDevice(deviceAddress));

            if (wp != null)
            {
                labelTemperature.Text = wp.temperature.ToString();
                labelHumidity.Text = wp.humidity.ToString();
                labelPressure.Text = wp.pressure.ToString();
            }
            else
            {
                // Handle the case where wp is null
                labelTemperature.Text = "N/A";
                labelHumidity.Text = "N/A";
                labelPressure.Text = "N/A";
            }
        }
        catch
        {
            return;
        }
    }
    public ShowData(bool hasAddress)
	{
		InitializeComponent();
        paramList = new List<WeatherParam>();
        if (hasAddress)
        {
            update();
        }
        else
        {
            //buttonResume.Enabled = false;
            //buttonSaveToFile.Enabled = false;
        }

    }




    private async Task updateChartAsync()
    {
        // pobieranie i updatowanie wykresu w tle
        try
        {
            WeatherParam wp = await Task.Run(() => WeatherParam.GetValuesFromDevice(deviceAddress));
            //chartWeather.Invoke((Action)(() =>
            //{
            //    chartWeather.Series["Temperature"].Points.AddXY(wp.date.Hour + ":" + wp.date.Minute, wp.temperature);
            //    chartWeather.Series["Humidity"].Points.AddXY(wp.date.Hour + ":" + wp.date.Minute, wp.humidity);
            //    chartWeather.Series["Pressure"].Points.AddXY(wp.date.Hour + ":" + wp.date.Minute, wp.pressure);
            //    paramList.Add(wp);
            //    //zmiana max i min dla osi Y
            //    chartWeather.ChartAreas["ChartTemperature"].AxisY.Maximum = paramList.Max(x => x.temperature) + 5;
            //    chartWeather.ChartAreas["ChartTemperature"].AxisY.Minimum = paramList.Min(x => x.temperature) - 5;
            //    chartWeather.ChartAreas["ChartHumidity"].AxisY.Maximum = paramList.Max(x => x.humidity) + 5;
            //    chartWeather.ChartAreas["ChartHumidity"].AxisY.Minimum = paramList.Min(x => x.humidity) - 5;
            //    chartWeather.ChartAreas["ChartPressure"].AxisY.Maximum = paramList.Max(x => x.pressure) + 5;
            //    chartWeather.ChartAreas["ChartPressure"].AxisY.Minimum = paramList.Min(x => x.pressure) - 5;

            //    // przerysowanie wykresu
            //    chartWeather.Invalidate();

            //    //dodanie danych do listViewWeather
            //    listViewWeather.Items.Add(new ListViewItem(new string[] { wp.date.Hour.ToString() + ":" + wp.date.Minute.ToString(), wp.temperature.ToString(), wp.humidity.ToString(), wp.pressure.ToString() }));

            //}));
        }
        catch
        {
            return;
        }
    }
    //private void setGraph()
    //{
    //    try
    //    {
    //        // tworzenie 3 serii dla danych wykresów
    //        chartWeather.Series.Add("Temperature");
    //        chartWeather.Series.Add("Humidity");
    //        chartWeather.Series.Add("Pressure");
    //    }
    //    catch { }

    //    // ustawienie typu wykresu 
    //    chartWeather.Series["Temperature"].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
    //    chartWeather.Series["Humidity"].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;
    //    chartWeather.Series["Pressure"].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Spline;

    //    //Wyg³adzenie linii
    //    chartWeather.Series["Temperature"]["LineTension"] = "0.3";
    //    chartWeather.Series["Humidity"]["LineTension"] = "0.3";
    //    chartWeather.Series["Pressure"]["LineTension"] = "0.2";


    //    //wyœwietlanie wartoœci na wykresie do maksymalnie 1 miejsca po przecinku
    //    chartWeather.ChartAreas["ChartTemperature"].AxisY.LabelStyle.Format = "{0:0.0}";
    //    chartWeather.ChartAreas["ChartHumidity"].AxisY.LabelStyle.Format = "{0:0.0}";
    //    chartWeather.ChartAreas["ChartPressure"].AxisY.LabelStyle.Format = "{0:0.0}";

    //    // przypisanie serii do chartArea
    //    chartWeather.Series["Temperature"].ChartArea = "ChartTemperature";
    //    chartWeather.Series["Humidity"].ChartArea = "ChartHumidity";
    //    chartWeather.Series["Pressure"].ChartArea = "ChartPressure";

    //    //ustawienie gruboœci linii
    //    chartWeather.Series["Temperature"].BorderWidth = 4;
    //    chartWeather.Series["Humidity"].BorderWidth = 4;
    //    chartWeather.Series["Pressure"].BorderWidth = 4;

    //    //ustawienie kolorów linii
    //    chartWeather.Series["Temperature"].Color = Color.Black;
    //    chartWeather.Series["Humidity"].Color = Color.Blue;
    //    chartWeather.Series["Pressure"].Color = Color.Green;

    //    //ustawienei max i min dla series
    //    chartWeather.ChartAreas["ChartTemperature"].AxisY.Maximum = 50;
    //    chartWeather.ChartAreas["ChartTemperature"].AxisY.Minimum = -20;
    //    chartWeather.ChartAreas["ChartHumidity"].AxisY.Maximum = 100;
    //    chartWeather.ChartAreas["ChartHumidity"].AxisY.Minimum = 0;
    //    chartWeather.ChartAreas["ChartPressure"].AxisY.Maximum = 1100;
    //    chartWeather.ChartAreas["ChartPressure"].AxisY.Minimum = 900;

    //    //stworzenie kolumn w listViewWeather
    //    listViewWeather.View = View.Details;
    //    listViewWeather.Columns.Add("", 40);
    //    listViewWeather.Columns.Add("", 40);
    //    listViewWeather.Columns.Add("", 50);

    //    //przedefiniowanie nazw kolumn
    //    listViewWeather.Columns[0].Text = "Data";
    //    listViewWeather.Columns[1].Text = "Temp";
    //    listViewWeather.Columns[2].Text = "Hum";
    //    listViewWeather.Columns[3].Text = "Pres";
    //}
    private async void DrawGraph()
    {
        stopExecuting = false;

        // setGraph();

        // pobieranie i udatowanie wykresu w tle w nieskoñczonej pêtli co jedn¹ sekundê
        while (!stopExecuting)
        {
            await updateChartAsync();
            await Task.Delay(1000); // czekaj jedn¹ sekundê
        }
    }

    //private void buttonSaveToFile_Click(object sender, EventArgs e)
    //{
    //    string path = "data.txt";
    //    //moœliwoœæ wyboru œcie¿ki zapisu
    //    SaveFileDialog saveFileDialog = new SaveFileDialog();
    //    saveFileDialog.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
    //    //saveFileDialog.FilterIndex = 1;
    //    saveFileDialog.RestoreDirectory = true;
    //    if (saveFileDialog.ShowDialog() == DialogResult.OK)
    //    {
    //        path = saveFileDialog.FileName;
    //    }

    //    try
    //    {
    //        using (System.IO.StreamWriter file = new System.IO.StreamWriter(path))
    //        {
    //            file.WriteLine("Data, Temperatura, Wilgotnoœæ, Ciœnienie");
    //            foreach (var param in paramList)
    //            {
    //                file.WriteLine(param.date + ", " + param.temperature + ", " + param.humidity + ", " + param.pressure);
    //            }
    //        }
    //    }
    //    catch
    //    {
    //        MessageBox.Show("Nie uda³o siê zapisaæ danych do pliku: " + path);
    //    }
    //    MessageBox.Show("Dane zosta³y zapisane do pliku: \n" + path);
    //}

    //private void buttonCloseApp_Click(object sender, EventArgs e)
    //{
    //    Application.Exit();
    //}

    //private void buttonReadFile_Click(object sender, EventArgs e)
    //{
    //    //Zatrzymanie pobierania danych
    //    stopExecuting = true;

    //    setGraph();
    //    //otwarcie okna dialogowego do wyboru pliku
    //    OpenFileDialog openFileDialog = new OpenFileDialog();
    //    openFileDialog.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
    //    openFileDialog.RestoreDirectory = true;
    //    string path = "";
    //    if (openFileDialog.ShowDialog() == DialogResult.OK)
    //    {
    //        path = openFileDialog.FileName;
    //    }
    //    //odczytanie pliku i dodanie danych do listy
    //    try
    //    {
    //        using (StreamReader sr = new StreamReader(path))
    //        {
    //            string line;
    //            while ((line = sr.ReadLine()) != null)
    //            {
    //                if (line != "Data, Temperatura, Wilgotnoœæ, Ciœnienie")
    //                {
    //                    string[] values = line.Split(',');
    //                    DateTime date = DateTime.Parse(values[0]);
    //                    float temperature = float.Parse(values[1]);
    //                    float humidity = float.Parse(values[2]);
    //                    float pressure = float.Parse(values[3]);
    //                    paramList.Add(new WeatherParam(date, temperature, humidity, pressure));
    //                }
    //            }
    //        }
    //    }
    //    catch
    //    {
    //        MessageBox.Show("Nie uda³o siê odczytaæ pliku: " + path);
    //        return;
    //    }

    //    //czyszczenie wykresu
    //    chartWeather.Series["Temperature"].Points.Clear();
    //    chartWeather.Series["Humidity"].Points.Clear();
    //    chartWeather.Series["Pressure"].Points.Clear();
    //    //dodanie danych do wykresu
    //    foreach (var param in paramList)
    //    {
    //        chartWeather.Series["Temperature"].Points.AddXY(param.date.Hour + ":" + param.date.Minute, param.temperature);
    //        chartWeather.Series["Humidity"].Points.AddXY(param.date.Hour + ":" + param.date.Minute, param.humidity);
    //        chartWeather.Series["Pressure"].Points.AddXY(param.date.Hour + ":" + param.date.Minute, param.pressure);
    //    }
    //    //zmiana max i min dla osi Y
    //    chartWeather.ChartAreas["ChartTemperature"].AxisY.Maximum = paramList.Max(x => x.temperature) + 5;
    //    chartWeather.ChartAreas["ChartTemperature"].AxisY.Minimum = paramList.Min(x => x.temperature) - 5;
    //    chartWeather.ChartAreas["ChartHumidity"].AxisY.Maximum = paramList.Max(x => x.humidity) + 5;
    //    chartWeather.ChartAreas["ChartHumidity"].AxisY.Minimum = paramList.Min(x => x.humidity) - 5;
    //    chartWeather.ChartAreas["ChartPressure"].AxisY.Maximum = paramList.Max(x => x.pressure) + 5;
    //    chartWeather.ChartAreas["ChartPressure"].AxisY.Minimum = paramList.Min(x => x.pressure) - 5;

    //    // przerysowanie wykresu
    //    chartWeather.Invalidate();

    //    //dodanie danych do listViewWeather
    //    listViewWeather.Items.Clear();
    //    foreach (var param in paramList)
    //    {
    //        listViewWeather.Items.Add(new ListViewItem(new string[] { param.date.Hour.ToString() + ":" + param.date.Minute.ToString(), param.temperature.ToString(), param.humidity.ToString(), param.pressure.ToString() }));
    //    }






    //}

    //private void buttonResume_Click(object sender, EventArgs e)
    //{
    //    //resume pobierania danych
    //    if (stopExecuting)
    //    {
    //        stopExecuting = false;
    //        DrawGraph();
    //    }


    //}
}