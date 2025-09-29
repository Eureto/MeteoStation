package mobileapp.MeteoStation

import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.content.Intent
import android.content.SharedPreferences
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.ListView
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat

class MainActivity : AppCompatActivity() {

    // Bluetooth & Scanning Properties
    private val bluetoothAdapter: BluetoothAdapter by lazy {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }
    private val bleScanner by lazy { bluetoothAdapter.bluetoothLeScanner }
    private var isScanning = false
    private val handler = Handler(Looper.getMainLooper())

    //  UI Properties
    private lateinit var scanButton: Button
    private lateinit var deviceListView: ListView
    private lateinit var deviceListAdapter: DeviceListAdapter

    // Data & State Properties
    private val discoveredDevices = mutableMapOf<String, ScanResultData>()
    private val SCAN_PERIOD: Long = 10000 // Stops scanning after 10 seconds.
    private lateinit var sharedPreferences: SharedPreferences
    // Permissions
    private val PERMISSIONS_REQUEST_CODE = 101
    private val requiredPermissions = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
        arrayOf(
            Manifest.permission.BLUETOOTH_SCAN,
            Manifest.permission.BLUETOOTH_CONNECT
        )
    } else {
        arrayOf(Manifest.permission.ACCESS_FINE_LOCATION)
    }

////////////////////////////////////////////////// Bluetooth Logic ////////////////////////////////////////////////////////////////////////

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        sharedPreferences = getSharedPreferences("ble_scanner_prefs", Context.MODE_PRIVATE)
        val savedDeviceAddress = sharedPreferences.getString("device_address", null)

        if (savedDeviceAddress != null) {
            // If a device is remembered, go directly to the device screen
            val intent = Intent(this, DeviceActivity::class.java)
            startActivity(intent)
            finish() // Finish MainActivity so user can't go back to it
            return
        }

        setContentView(R.layout.activity_main)

        // Initialize UI components
        scanButton = findViewById(R.id.scanButton)
        deviceListView = findViewById(R.id.deviceListView)

        // Setup the custom adapter for the list view
        deviceListAdapter = DeviceListAdapter(this, mutableListOf())
        deviceListView.adapter = deviceListAdapter

        scanButton.setOnClickListener {
            if (isScanning) {
                stopBleScan()
            } else {
                startBleScan()
            }
        }
        deviceListView.setOnItemClickListener { _, _, position, _ ->
            stopBleScan()
            val selectedDevice = deviceListAdapter.getItem(position) ?: return@setOnItemClickListener
            val deviceAddress = selectedDevice.deviceAddress
            val deviceName = selectedDevice.deviceName

            // Save the device address and name
            with(sharedPreferences.edit()) {
                putString("device_address", deviceAddress)
                putString("device_name", deviceName)
                // Use commit() for a synchronous save to prevent race conditions before starting the next activity
                commit()
            }

            // Start the DeviceActivity
            val intent = Intent(this, DeviceActivity::class.java)
            startActivity(intent)
            finish()
        }
    }

    override fun onResume() {
        super.onResume()
        if (!bluetoothAdapter.isEnabled) {
            // Consider prompting user to enable Bluetooth
            Toast.makeText(this, "Please enable Bluetooth", Toast.LENGTH_SHORT).show()
        }
    }

    // BLE Scanning Logic
    private fun startBleScan() {
        if (requiredPermissions.any { ContextCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED }) {
            ActivityCompat.requestPermissions(this, requiredPermissions, PERMISSIONS_REQUEST_CODE)
            return
        }
        performScan()
    }
    private fun performScan() {

        if(ActivityCompat.checkSelfPermission(this, requiredPermissions.first()) != PackageManager.PERMISSION_GRANTED) {
            Toast.makeText(this, "Permissions are not granted.", Toast.LENGTH_SHORT).show()
            return
        }

        requestBluetoothEnableIfNeeded()

        if (isScanning) return
        discoveredDevices.clear()
        deviceListAdapter.clear()

        handler.postDelayed({ stopBleScan() }, SCAN_PERIOD)
        isScanning = true
        scanButton.text = "Stop Scan"
        bleScanner.startScan(null, scanSettings, scanCallback)
    }
    private fun stopBleScan() {
        if (!isScanning) return // Already stopped

        isScanning = false
        scanButton.text = "Start Scan"
        if (ActivityCompat.checkSelfPermission(this, requiredPermissions.first()) == PackageManager.PERMISSION_GRANTED) {
            bleScanner.stopScan(scanCallback)
        }
    }

    private val scanSettings = ScanSettings.Builder()
        .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
        .build()

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {

            if (!discoveredDevices.containsKey(result.device.address)) {
                // We need BLUETOOTH_CONNECT permission for the device name on newer Android versions
                val deviceName = if (ActivityCompat.checkSelfPermission(this@MainActivity, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED) {
                    result.device.name ?: "Unnamed"
                } else {
                    result.device.name ?: "Unnamed"
                }

                // val rawHexData = result.scanRecord?.bytes?.joinToString("") { "%02X".format(it) } ?: "N/A"
                // val asciiData = result.scanRecord?.bytes?.map { it.toInt().toChar() }?.joinToString("") ?: "N/A" // Simplified, might need filtering for printable chars


                // Create an instance of ScanResultData
                val scanResultData = ScanResultData(
                    deviceName = deviceName,
                    deviceAddress = result.device.address
                )

                // Store the ScanResultData instance in the map
                discoveredDevices[result.device.address] = scanResultData
                deviceListAdapter.add(scanResultData)
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e("ScanCallback", "onScanFailed: code $errorCode")
        }
    }
    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == PERMISSIONS_REQUEST_CODE) {
            if (grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
                performScan()
            } else {
                Toast.makeText(this, "Permissions are required for scanning.", Toast.LENGTH_LONG).show()
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
}

// Data Class and Custom Adapter
/**
 * Data class to hold the processed information from a scan result.
 */
data class ScanResultData(
    val deviceName: String,
    val deviceAddress: String
)

/**
 * Custom adapter to display the scan results in a more detailed format.
// */
class DeviceListAdapter(
    context: Context,
    private var dataSource: List<ScanResultData>
) : ArrayAdapter<ScanResultData>(context, android.R.layout.simple_list_item_2, dataSource) {

    override fun clear() {
        this.dataSource = emptyList()
        notifyDataSetChanged()
    }
    fun add(item: ScanResultData) {
        this.dataSource = this.dataSource + item
        notifyDataSetChanged()
    }
    fun updateData(newData: List<ScanResultData>) {
        this.dataSource = newData
        notifyDataSetChanged()
    }

    override fun getCount(): Int {
        return dataSource.size
    }

    override fun getItem(position: Int): ScanResultData? {
        return dataSource[position]
    }

    override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
        val view: View
        val viewHolder: ViewHolder

        if (convertView == null) {
            view = View.inflate(context, android.R.layout.simple_list_item_2, null)
            viewHolder = ViewHolder(
                view.findViewById(android.R.id.text1),
                view.findViewById(android.R.id.text2)
            )
            view.tag = viewHolder
        } else {
            view = convertView
            viewHolder = view.tag as ViewHolder
        }

        val item = getItem(position)
        val deviceName = item?.deviceName ?: "Unknown"
        val deviceAddress = item?.deviceAddress ?: "No Address"

        viewHolder.text1.text = "$deviceName"
        viewHolder.text1.textSize = 22f
        viewHolder.text2.text = "$deviceAddress"
        viewHolder.text2.textSize = 12f
        return view
    }

    private class ViewHolder(val text1: TextView, val text2: TextView)

}
