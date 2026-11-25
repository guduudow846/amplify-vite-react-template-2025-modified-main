# 🌡️ IoT Weather & Environment Monitoring System

### Smart Environmental Monitoring with ESP32, AWS Cloud, SMHI & Discord

An end-to-end IoT project that connects physical sensors to the cloud and transforms environmental data into useful information through a web dashboard.

<p align="center">

**ESP32 · MQTT · AWS IoT Core · Lambda · Amplify · SMHI · Discord**

</p>

---

## 📌 About the Project

This project was developed to demonstrate how an IoT solution can help property owners monitor indoor environmental conditions remotely.

The system collects temperature and humidity readings using a DHT11 sensor connected to an Arduino Nano ESP32. The measurements are sent over Wi-Fi using MQTT to AWS IoT Core, processed by AWS Lambda, and stored in AWS Amplify Data.

The project also integrates weather observations from the SMHI Open Data API and provides a web-based dashboard where collected information can be viewed.

Discord webhook integration provides an additional way to receive system messages.

### ✨ Project Highlights

- 🌡️ Temperature and humidity monitoring.
- 📡 Wireless communication using Wi-Fi and MQTT.
- ☁️ Cloud processing using AWS services.
- 🗄️ Centralized data storage using AWS Amplify Data.
- 🌍 External weather data from SMHI.
- 📊 Web dashboard built with React and Vite.
- 💬 Discord webhook notifications.
- 🔐 Authentication and access control using Amazon Cognito and Amplify authorization.

---

## 🎯 1. Business Need

Property owners need a simple way to understand the environmental conditions inside their buildings.

Temperature and humidity can affect indoor comfort and building conditions. Without remote monitoring, property owners may need to physically inspect rooms to understand what is happening.

This project provides a solution where environmental measurements can be collected automatically, stored in the cloud, and accessed through a web application.

**The main objectives are:**

- Monitor indoor temperature and humidity remotely.
- Store measurements for historical analysis.
- Compare indoor sensor readings with external weather information.
- Provide access to collected data through a dashboard.
- Support notifications through Discord.

---

## 🏗️ 2. System Architecture

The solution consists of a physical sensor, wireless communication, AWS cloud services, external API integration, and a web dashboard.

### Architecture Diagram

```mermaid
flowchart TD
    A[DHT11 Sensor] --> B[Arduino Nano ESP32]
    B -->|Wi-Fi / MQTT| C[AWS IoT Core]
    C --> D[AWS IoT Rule]
    D --> E[Lambda: graphqlIoTCoreTelemetry]
    E --> F[AWS Amplify Data]
    F --> G[React / Vite Dashboard]

    H[Amazon EventBridge] --> I[Lambda: pollSmhi]
    J[SMHI Open Data API] --> I
    I --> F

    E --> K[Discord Webhook]
    L[Amazon Cognito] --> G
```

### Data Flow

1. The DHT11 sensor measures temperature and humidity.
2. The Arduino Nano ESP32 reads the sensor values.
3. The ESP32 publishes the measurements using MQTT over Wi-Fi.
4. AWS IoT Core receives the messages.
5. An AWS IoT Rule forwards the data to a Lambda function.
6. Lambda processes the incoming data and stores it in Amplify Data.
7. The React dashboard retrieves and displays the stored measurements.
8. A separate Lambda function retrieves weather observations from SMHI.
9. Discord can receive messages through a configured webhook.

---

## 🔌 3. Hardware and Communication

### Hardware Components

| Component | Purpose |
|---|---|
| Arduino Nano ESP32 | Main IoT microcontroller |
| DHT11 | Temperature and humidity sensor |
| Wi-Fi | Wireless internet connection |
| USB connection | Programming and power |

### Sensor Communication

The DHT11 sensor is connected to the Arduino Nano ESP32.

The ESP32 reads the measurements and sends them to AWS IoT Core using MQTT.

**MQTT Topic:**

```text
esp32-hassan/telemetry
```

**Example MQTT Message:**

```json
{
  "temperature": 22.1,
  "humidity": 30
}
```

MQTT is a lightweight publish/subscribe communication protocol commonly used in IoT systems.

It allows devices to publish messages without needing a direct connection to the application that consumes the data.

---

## ☁️ 4. AWS Cloud Infrastructure

The project uses several AWS services to process, store, secure, and deliver IoT data.

| AWS Service | Responsibility |
|---|---|
| AWS IoT Core | Receives MQTT messages |
| AWS IoT Rules | Routes incoming sensor data |
| AWS Lambda | Processes telemetry and weather data |
| AWS Amplify Data | Stores and provides access to application data |
| Amazon Cognito | User authentication |
| Amazon EventBridge | Scheduled SMHI data collection |
| Amazon DynamoDB | Tested as an alternative storage approach |

### AWS Lambda Functions

The project uses Lambda functions for different tasks:

- `graphqlIoTCoreTelemetry` – processes ESP32 telemetry and supports Discord webhook messages.
- `graphqlIoTCoreStatus` – processes device status updates.
- `pollSmhi` – retrieves weather observations from SMHI and stores them in Amplify Data.

This serverless approach reduces the need to manage dedicated servers.

---

## 🌍 5. External API Integration – SMHI

The system integrates with the **SMHI Open Data API** to retrieve external weather observations.

A Lambda function retrieves temperature measurements from SMHI weather station `72420`.

The collected weather data is stored in Amplify Data and can be viewed in the dashboard.

### Why SMHI?

SMHI provides external weather observations that can be used alongside the project's own sensor readings.

This creates opportunities to compare indoor environmental measurements with outdoor weather conditions.

---

## 💬 6. Discord Integration

The project includes Discord webhook integration.

The Lambda functions can send telemetry and device-status messages to a configured Discord channel.

This provides an additional communication channel where system information can be received without opening the dashboard.

**Example workflow:**

```text
ESP32 → AWS IoT Core → Lambda → Discord Webhook
```

Webhook URLs are treated as secrets and are not included in this repository.

---

## 📊 7. Data Storage and Visualization

An important part of this project is the separation between data storage and data visualization.

### Data Storage

AWS Amplify Data provides the GraphQL data layer used to store:

- Device information.
- Device identifiers.
- Timestamps.
- Temperature readings.
- Humidity readings when available.
- Owner information.

Amazon DynamoDB was also tested separately as an alternative way to store IoT measurements.

### Data Visualization

The web application is developed using:

- React
- Vite
- AWS Amplify
- GraphQL
- Amazon Cognito

The dashboard retrieves stored telemetry and presents it in a user-friendly interface.

**Storage is responsible for keeping the data, while visualization is responsible for presenting the data to users.**

---

## 🔐 8. Security

Security is important because IoT devices communicate with cloud services and transmit environmental information.

The project includes the following security measures:

- Amazon Cognito authentication for dashboard users.
- Owner-based authorization for Devices and Telemetry data.
- IAM permissions for Lambda functions accessing Amplify Data.
- AWS IoT Core for MQTT communication.
- HTTPS connections to external APIs and webhooks.
- Public API-key access rules removed from the Devices and Telemetry models.
- Webhook URLs kept private and excluded from public documentation.

### Security Considerations

For a larger deployment, additional security improvements could include:

- Strong device identity and certificate lifecycle management.
- More detailed IAM least-privilege policies.
- Validation of incoming device messages.
- Monitoring and auditing of access.
- Secure secret management for external integrations.

---

## 📈 9. Scalability

The architecture is based on managed and serverless cloud services.

This makes it possible to expand the solution without redesigning the entire system.

### Possible Scaling Strategy

**More IoT Devices**

Additional ESP32 devices can be registered using unique device identifiers.

**Cloud Processing**

AWS IoT Core can receive messages from multiple devices, while Lambda functions process incoming events.

**Centralized Storage**

Amplify Data provides a central data layer for device information and measurements.

**Dashboard Expansion**

The React dashboard can be extended to display multiple devices, historical readings, and additional environmental measurements.

### Future Scalability Improvements

For a larger deployment, the architecture could be improved with:

- Amazon SQS for message buffering.
- Improved error handling and retry mechanisms.
- CloudWatch monitoring and alarms.
- Data-retention policies.
- Optimized database queries.
- Controlled Lambda concurrency.

These improvements would help support a larger number of connected devices and higher message volumes.

---

## 🤖 10. Future Development and AI Opportunities

The project provides a foundation for additional IoT functionality.

Possible future improvements include:

- Adding more environmental sensors.
- Configurable temperature and humidity thresholds.
- Automatic alerts when measurements exceed configured limits.
- More detailed historical charts.
- Predictive analysis of environmental conditions.
- Machine learning for detecting unusual temperature and humidity patterns.

For example, a future machine-learning model could analyze historical measurements and identify unusual changes that may indicate ventilation or environmental problems.

These AI capabilities are future possibilities and are not currently implemented.

---

## 🧪 11. Testing and Verification

The following integrations have been tested during development:

| Feature | Verification |
|---|---|
| ESP32 sensor telemetry | Temperature and humidity visible in dashboard |
| AWS IoT Core | MQTT telemetry routed through IoT Rule |
| AWS Lambda | Telemetry processing tested |
| Amplify Data | Sensor records displayed in dashboard |
| SMHI integration | Weather station data displayed in dashboard |
| Discord integration | Webhook message successfully received |
| Authentication | Dashboard login tested |
| Data authorization | Owner-based authorization deployed |

---

## 🛠️ 12. Technologies Used

| Category | Technology |
|---|---|
| Hardware | Arduino Nano ESP32 |
| Sensor | DHT11 |
| Communication | Wi-Fi, MQTT |
| Cloud | AWS IoT Core |
| Processing | AWS Lambda |
| Data layer | AWS Amplify Data |
| Authentication | Amazon Cognito |
| Scheduling | Amazon EventBridge |
| External API | SMHI Open Data |
| Notifications | Discord Webhooks |
| Frontend | React and Vite |
| API | GraphQL |
| Version control | Git and GitHub |

---

## 🎓 13. Learning Outcomes

Through this project, I have gained practical experience with:

- Connecting physical sensors to an ESP32.
- Establishing wireless IoT communication.
- Understanding MQTT publish/subscribe architecture.
- Configuring AWS IoT Core and IoT Rules.
- Processing data using AWS Lambda.
- Storing and retrieving cloud data.
- Integrating external APIs.
- Building a web dashboard for IoT visualization.
- Implementing authentication and authorization.
- Understanding scalability and security in cloud-based IoT systems.

---

## 🏁 14. Conclusion

The IoT Weather & Environment Monitoring System demonstrates how hardware, wireless communication, cloud services, external APIs, and web technologies can work together in a complete IoT solution.

The project connects an ESP32 sensor to AWS IoT Core, processes measurements through Lambda, stores data in Amplify Data, retrieves weather observations from SMHI, and presents information through a React dashboard.

It also demonstrates how cloud-based IoT solutions can be designed with security, scalability, and future development in mind.

**This project is a practical demonstration of an end-to-end IoT architecture, from sensor to cloud to dashboard.**

---

### 👨‍💻 Project Repository

Developed as an IoT portfolio project using Arduino Nano ESP32, AWS Cloud Services, and React.
