# Sensor Node Configuration

## 1. Purpose

Defines the compile-time configuration for a WeatherBus Sensor Node, including its Node ID, I2C settings, sensor selection, sensor addresses, and Base Station MAC address.

Source: `src/config/node/node_settings.h`

## 2. Node Identity

| Constant | Value | Description |
|---|---|---|
| `NODE_ID` | `1` | Identifier assigned to this Sensor Node |

`NODE_ID` must match the corresponding Node ID configured in the Base Station's `node_config.h`.

## 3. I2C Configuration

| Constant | Value | Description |
|---|---:|---|
| `I2C_SDA` | `8` | I2C SDA pin |
| `I2C_SCL` | `9` | I2C SCL pin |
| `I2C_CLOCK` | `100000` Hz | I2C clock frequency (100 kHz) |

These constants define the I2C settings used by the Node firmware.

## 4. Sensor Selection

| Constant | Value | Description |
|---|---|---|
| `ENABLE_SHT41` | `false` | SHT41 sensor disabled |
| `ENABLE_BME280` | `true` | BME280 sensor enabled |
| `USE_BME280_TEMPERATURE` | `true` | Select BME280 for temperature |
| `USE_BME280_HUMIDITY` | `true` | Select BME280 for humidity |

These values configure sensor availability and the intended source for temperature and humidity. Actual sensor initialization and reading behaviour are implemented in the relevant source files.

## 5. SHT41 Configuration

| Constant | Value | Description |
|---|---:|---|
| `SHT41_ADDRESS` | `0x44` | I2C address |
| `SHT41_MEASURE_HIGH_PRECISION` | `0xFD` | High-precision measurement command |

Although these constants are defined, SHT41 is currently disabled by `ENABLE_SHT41`.

## 6. BME280 Configuration

| Constant | Value | Description |
|---|---:|---|
| `BME280_I2C_ADDRESS` | `0x76` | I2C address |

BME280 is currently enabled.

## 7. Base Station MAC Address

`baseMac` stores the six-byte MAC address of the Base Station:

`0C:4E:A0:4D:7C:40`

The Node uses this address when configuring communication with the Base Station. Verify the actual usage in the transport implementation.

## 8. Development Constraints

- Keep `NODE_ID` consistent with the Base Station configuration.
- Confirm the I2C pins and addresses match the hardware.
- Check sensor enable flags and source-selection flags when changing sensor configuration.
- Ensure `baseMac` matches the intended Base Station.
- Rebuild and test the Node firmware after changing configuration.

## 9. Implementation Notes

This header defines compile-time constants only. It does not initialize I2C, initialize sensors, read measurements, or establish ESP-NOW communication.

## 10. Test Status

The configuration values alone do not establish that the selected sensor or communication link has been successfully tested. Record verification results separately.