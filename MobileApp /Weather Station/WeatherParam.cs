using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;

namespace ProjektDotNet
{
    internal class WeatherParam
    {
        public DateTime date;
        public float temperature;
        public float humidity;
        public float pressure;

        // constructor
        public WeatherParam(DateTime date, float temperature, float humidity, float pressure)
        {
            this.date = date;
            this.temperature = temperature;
            this.humidity = humidity;
            this.pressure = pressure;
        }

        private static readonly HttpClient hc = new HttpClient();

        public static string DownloadHistoryData(string url)
        {
            int attempts = 0;
            const int maxAttempts = 3;
        Retry:

            try
            {
                url += "/list";
                // towrzenie obiektu do wysyłania zapytania na podany adres 
                HttpRequestMessage request = new HttpRequestMessage(HttpMethod.Get, url);
                HttpResponseMessage response = hc.SendAsync(request).Result;
                response.EnsureSuccessStatusCode();
                // get response body test as string 
                string responseBody = hc.GetStringAsync(url).Result;
                return responseBody;
            }
            catch (Exception)
            {
                attempts++;
                Task.Delay(100).Wait();
                if (attempts >= maxAttempts)
                {
                    return null;
                }
                goto Retry;
            }


        }
        public static WeatherParam GetValuesFromDevice(string url)
        {
            int attempts = 0;
            const int maxAttempts = 3;
            Retry:
           
                try
                {
                // towrzenie obiektu do wysyłania zapytania na podany adres 
                    HttpRequestMessage request = new HttpRequestMessage(HttpMethod.Head, url);
                    HttpResponseMessage response = hc.SendAsync(request).Result;

                    response.EnsureSuccessStatusCode();
                    if (response.Headers.TryGetValues("Location", out var headerValues))
                    {
                        string headerMessage = headerValues.First();
                        // podział odpowiedzi do zmiennych
                        string[] values = headerMessage.Split('&');
                        float temperature = 0;
                        float humidity = 0;
                        float pressure = 0;
                        foreach (var value in values)
                        {
                            string[] valueSplit = value.Split('=');
                            switch (valueSplit[0])
                            {
                                case "temperature":
                                    temperature = float.Parse(valueSplit[1]);
                                    break;
                                case "humidity":
                                    humidity = float.Parse(valueSplit[1]);
                                    break;
                                case "pressure":
                                    pressure = float.Parse(valueSplit[1]);
                                    break;
                            }
                        }

                        return new WeatherParam(DateTime.Now, temperature, humidity, pressure);
                    }
                }
                catch (Exception)
                {
                    attempts++;
                    Task.Delay(100).Wait();
                    if (attempts >= maxAttempts)
                        {
                            return null;
                        }
                    goto Retry;
                }
            

            // W wypadku gdyby wystąpił nieoczekiwany błąd
            throw new InvalidOperationException("Failed to get values from device after multiple attempts.");
        }
    }
}
