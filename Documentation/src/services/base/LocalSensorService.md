LocalSensorService
1. Purpose
Provides a service interface for initializing a local sensor source and reading its measurements into a WeatherBus sensor payload.

Source: src/services/LocalSensorService.cpp
Header: src/services/LocalSensorService.h
2. Sensor Source
LocalSensorService receives an ISensorSource reference through its constructor.

The service delegates sensor initialization and reading to this source rather than implementing sensor-specific operations itself.
3. Initialization
The begin() method calls:

sensorSource.begin()

It returns the result directly. The service does not add separate initialization checks.
4. Sensor Reading
readSensors(data, flags) performs the following operations:
1. Creates a SensorReading object.
2. Initializes data.temperature, data.humidity, and data.pressure to 0.0f.
3. Initializes flags to 0.
4. Calls sensorSource.read(reading).
5. Returns false immediately if the source read fails.
6. Copies each valid measurement into the payload and sets its corresponding validity flag.
5. Measurement Validity
Reading field	Payload field	WeatherBus flag
temperatureValid	data.temperature	FLAG_TEMPERATURE_VALID
humidityValid	data.humidity	FLAG_HUMIDITY_VALID
pressureValid	data.pressure	FLAG_PRESSURE_VALID


Each measurement is checked independently. If a reading is invalid, its payload field remains 0.0f and its validity flag is not set.
6. Return Value
After processing the readings, readSensors() returns:
- true if at least one validity flag is set.
- false if no validity flags are set.
A failed sensorSource.read(reading) also causes an immediate false return.
7. Dependencies
- LocalSensorService.h
- Arduino.h
- ../common/weatherbus_protocol.h
- ../../interfaces/ISensorSource.h
8. Implementation Notes
LocalSensorService does not select a specific sensor, configure I2C, or validate physical measurement ranges. It relies on the provided ISensorSource implementation to perform the read and indicate which measurements are valid.
9. Test Status
The source code describes the service behaviour but does not, by itself, establish that the service or connected sensors have passed hardware or integration testing. Record verification results separately.



TREE FLOW

LocalSensorService
│
├── Constructor(sensorSource)
│   └── Store ISensorSource reference
│
├── begin()
│   └── Call sensorSource.begin()
│       └── Return initialization result
│
└── readSensors(data, flags)
    │
    ├── Initialize sensor data
    │   ├── temperature = 0.0
    │   ├── humidity = 0.0
    │   ├── pressure = 0.0
    │   └── flags = 0
    │
    ├── Call sensorSource.read(reading)
    │   ├── Failed
    │   │   └── Return false
    │   └── Successful
    │       └── Continue processing sensor readings
    │
    ├── Check temperatureValid
    │   ├── Valid
    │   │   ├── Copy temperature value
    │   │   └── Set FLAG_TEMPERATURE_VALID
    │   └── Invalid → Keep default value
    │
    ├── Check humidityValid
    │   ├── Valid
    │   │   ├── Copy humidity value
    │   │   └── Set FLAG_HUMIDITY_VALID
    │   └── Invalid → Keep default value
    │
    ├── Check pressureValid
    │   ├── Valid
    │   │   ├── Copy pressure value
    │   │   └── Set FLAG_PRESSURE_VALID
    │   └── Invalid → Keep default value
    │
    └── Check flags
        ├── flags != 0 → Return true
        └── flags == 0 → Return false