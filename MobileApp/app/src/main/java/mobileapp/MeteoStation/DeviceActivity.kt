package mobileapp.MeteoStation


import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.content.Intent
import android.content.SharedPreferences
import android.content.pm.PackageManager
import android.graphics.Color
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresPermission
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import java.nio.charset.StandardCharsets
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class DeviceActivity : AppCompatActivity() {

    // --- Bluetooth & Scanning ---
    private val bluetoothAdapter: BluetoothAdapter by lazy {
        (getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager).adapter
    }
    private val bleScanner by lazy { bluetoothAdapter.bluetoothLeScanner }
    private var isScanning = false
    private val handler = Handler(Looper.getMainLooper())
    private val REFRESH_INTERVAL: Long = 2500
    //private val SCAN_DURATION: Long = 4000

    // --- Device Info ---
    private var deviceAddress: String? = null
    private var deviceName: String? = null
    private lateinit var sharedPreferences: SharedPreferences

    // --- UI Elements ---
    private lateinit var deviceNameLabel: TextView
    private lateinit var connectionStatusLabel: TextView
    private lateinit var lastUpdateLabel: TextView
    private lateinit var temperatureLabel: TextView
    private lateinit var humidityLabel: TextView
    private lateinit var pressureLabel: TextView
    private lateinit var tempChart: LineChart
    private lateinit var humidityChart: LineChart
    private lateinit var pressureChart: LineChart // New chart
    private lateinit var forgetDeviceButton: Button

    // --- Chart Data ---
    private val tempData = mutableListOf<Entry>()
    private val humidityData = mutableListOf<Entry>()
    private val pressureData = mutableListOf<Entry>() // New data list
    private var dataCounter = 0f

    // --- Permissions ---
    private val PERMISSIONS_REQUEST_CODE = 102 // Use a different code than MainActivity
    private val requiredPermissions = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
        // On android 12+
        arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT)
    } else {
        // On older APIs, connect permission is implied by admin, and scan needs location.
        arrayOf(Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.BLUETOOTH_ADMIN)
    }

    fun handlePermissions(){
        //while permissions are not granted, request them. Repeat until granted (interval of 2 seconds)
        while(ActivityCompat.checkSelfPermission(this, requiredPermissions.first()) == PackageManager.PERMISSION_DENIED) {
            // Request the missing permissions
            ActivityCompat.requestPermissions(this, requiredPermissions, PERMISSIONS_REQUEST_CODE)
            // Wait for 2 seconds before checking again
            Thread.sleep(2000)
        }

    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.device_activity_layout)

        sharedPreferences = getSharedPreferences("ble_scanner_prefs", Context.MODE_PRIVATE)
        deviceAddress = sharedPreferences.getString("device_address", null)
        deviceName = sharedPreferences.getString("device_name", "Unknown Device")

        if (deviceAddress == null) {
            goBackToScan()
            return
        }

        initializeUI()
        setupCharts()
        startTargetedScan()
    }

//    private val periodicScanRunnable = object : Runnable {
//        override fun run() {
//            if(!isScanning){
//                startTargetedScan()
//                Log.i("DeviceActivity", "Periodic scan triggered" )
//            }
//            handler.postDelayed(this, REFRESH_INTERVAL)
//        }
//    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
    override fun onResume() {
        super.onResume()
        startTargetedScan()

        //handler.post(periodicScanRunnable)
    }

    override fun onPause() {
        super.onPause()
        // handler.removeCallbacks(periodicScanRunnable)
        stopBleScan()
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
    private fun startTargetedScan() {
        // Redundant safety check
        if(isScanning) return

        handlePermissions()
        requestBluetoothEnableIfNeeded()

        val scanFilter = ScanFilter.Builder().setDeviceAddress(deviceAddress).build()
        val scanSettings = ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build()

        connectionStatusLabel.text = "Status: Scanning..."
        isScanning = true

        // handler.postDelayed({ stopBleScan() }, SCAN_DURATION)
        bleScanner.startScan(listOf(scanFilter), scanSettings, scanCallback)
    }

    private fun stopBleScan() {
        if (!isScanning) return

        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED) {
                    bleScanner.stopScan(scanCallback)
                } else {
                    Log.w("DeviceActivity", "BLUETOOTH_SCAN permission not granted when trying to stop scan (API 31+).")
                    // The scan might have already been implicitly stopped by the system if permission was revoked.
                }
            } else {
                // For API 30 and below, if the scan was started, ACCESS_FINE_LOCATION was granted.
                // StopScan itself doesn't typically require a new permission check distinct from the one needed to start it.
                bleScanner.stopScan(scanCallback)
            }
        } catch (e: SecurityException) {
            Log.e("DeviceActivity", "SecurityException while stopping scan: ${e.message}")
            // This might happen if permissions were revoked in a very specific way.
        } finally {
            connectionStatusLabel.text = "Status: Stopped Ble Scan"
            isScanning = false
            Log.d("DeviceActivity", "Scan stopped. isScanning = $isScanning")
            // Update UI or state as needed
        }
    }

    private val scanCallback = object : ScanCallback() {
        @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
        override fun onScanResult(callbackType: Int, result: ScanResult) {

            stopBleScan()
            // after REFRESH_INTERVAL start scanning again
            handler.postDelayed({ startTargetedScan() }, REFRESH_INTERVAL)

            connectionStatusLabel.text = "Status: Device Found"
            Log.d("DeviceActivity", "Found device: ${result.device.address}")

            // The data is now in the device name, so we need permission to read it.
            if (ActivityCompat.checkSelfPermission(this@DeviceActivity, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                return
            }
            val advertisementData = result.scanRecord?.getBytes();
            processAdvertisementData(advertisementData)
        }
        @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
        override fun onScanFailed(errorCode: Int) {
            stopBleScan()
            handler.postDelayed({startTargetedScan() }, REFRESH_INTERVAL)

            Log.e("DeviceActivity", "Scan Failed with code: $errorCode")
            connectionStatusLabel.text = "Status: Scan Failed"
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == PERMISSIONS_REQUEST_CODE) {
            if (grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
                Toast.makeText(this, "Permissions granted. Starting scan.", Toast.LENGTH_SHORT).show()
            } else {
                Toast.makeText(this, "Permissions are required to get device data.", Toast.LENGTH_LONG).show()
                connectionStatusLabel.text = "Status: Permissions denied"
            }
        }
    }

    fun requestBluetoothEnableIfNeeded(){
        while(!bluetoothAdapter.isEnabled)
        {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            requestBluetoothEnable.launch(enableBtIntent)
            // After 4 seconsds check again if bluetooth is enabled
            Thread.sleep(4000)
        }
    }
    private val requestBluetoothEnable =
        registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
            if (result.resultCode == Activity.RESULT_OK) {
                // Bluetooth was enabled by the user
                // Proceed with your Bluetooth operations
                Log.d("Bluetooth", "Bluetooth enabled by user.")
            } else {
                // User denied or cancelled enabling Bluetooth
                Log.d("Bluetooth", "User did not enable Bluetooth.")
                Toast.makeText(this, "Bluetooth is required to connect to device.", Toast.LENGTH_SHORT).show()
            }
        }

    /////////////////////////////// UI & Data Processing ///////////////////////////////
    /**
     * Parses the custom string format: "Temp_25.62Hum_40.55Pres_997.0 MeteoStation"
     */
    private fun initializeUI() {
        deviceNameLabel = findViewById(R.id.deviceNameLabel)
        lastUpdateLabel = findViewById(R.id.lastUpdateLabel)
        connectionStatusLabel = findViewById(R.id.connectionStatusLabel)
        temperatureLabel = findViewById(R.id.temperatureLabel)
        humidityLabel = findViewById(R.id.humidityLabel)
        pressureLabel = findViewById(R.id.pressureLabel)
        tempChart = findViewById(R.id.tempChart)
        humidityChart = findViewById(R.id.humidityChart)
        pressureChart = findViewById(R.id.pressureChart) // Initialize new chart
        forgetDeviceButton = findViewById(R.id.forgetDeviceButton)

        deviceNameLabel.text = deviceName

        forgetDeviceButton.setOnClickListener {
            with(sharedPreferences.edit()) {
                remove("device_address")
                remove("device_name")
                apply()
            }
            goBackToScan()
        }
    }

    private fun processAdvertisementData(data: ByteArray?) {

        //Debugging
        val hexString = data?.joinToString(" ") { String.format("%02X", it) }
        //Log.d("DeviceActivity", "Advertisement data (hex): $hexString")

        if (data == null) {
            Log.w("DataProcessing", "Advertisement data is null")
            connectionStatusLabel.text = "Status: No data"
            return
        }

        try{
            val dataString: String
            dataString = String(data, StandardCharsets.UTF_8).trim()
            //Log.d("DeviceActivity", "Advertisement data (string): $dataString")

            val tempString = dataString.substringAfter("Temp_").substringBefore("Hum_")
            val humString = dataString.substringAfter("Hum_").substringBefore("Pres_")
            // Take the part after "Pres_" and before the first space or end of string.
            val presString = dataString.substringAfter("Pres_").substringBefore("MeteoStation")

            val temperature = tempString.toFloatOrNull() ?: 0f
            val humidity = humString.toFloatOrNull() ?: 0f
            val pressure = presString.toFloatOrNull() ?: 0f

            Log.i("DataProcessing", "Temp: $temperature, Humidity: $humidity, Pressure: $pressure")
            updateUI(temperature, humidity, pressure)

        } catch (e: Exception) {
            Log.e("DataProcessing", "Error parsing advertisement string", e)
            connectionStatusLabel.text = "Status: Error parsing data"
        }
    }

    private fun updateUI(temp: Float, hum: Float, pres: Float) {
        temperatureLabel.text = String.format("%.2f °C", temp)
        humidityLabel.text = String.format("%.2f %%", hum)
        pressureLabel.text = String.format("%.1f hPa", pres)
        val currentTime = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date())
        lastUpdateLabel.text = "Last Update: $currentTime"

        dataCounter++
        tempData.add(Entry(dataCounter, temp))
        humidityData.add(Entry(dataCounter, hum))
        pressureData.add(Entry(dataCounter, pres)) // Add pressure data

        updateChart(tempChart, tempData, "Temperature", Color.RED)
        updateChart(humidityChart, humidityData, "Humidity", Color.BLUE)
        updateChart(pressureChart, pressureData, "Pressure", Color.DKGRAY) // Update pressure chart
    }

    private fun setupCharts() {
        configureChart(tempChart, "Temperature Data")
        configureChart(humidityChart, "Humidity Data")
        configureChart(pressureChart, "Pressure Data") // Configure pressure chart
    }

    private fun configureChart(chart: LineChart, description: String) {
        chart.description.text = description
        chart.setNoDataText("Waiting for data...")
        chart.invalidate()
    }

    private fun updateChart(chart: LineChart, data: List<Entry>, label: String, color: Int) {
        val dataSet = LineDataSet(data, label)
        dataSet.color = color
        dataSet.valueTextColor = Color.BLACK
        dataSet.setDrawCircles(false)
        dataSet.setDrawValues(false)

        chart.data = LineData(dataSet)
        chart.notifyDataSetChanged()
        chart.invalidate()
    }

    private fun goBackToScan() {
        val intent = Intent(this, MainActivity::class.java)
        intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
        startActivity(intent)
        finish()
    }
}