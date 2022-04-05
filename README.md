# Pet Feeder
IoT individual project 2022

The project was born from the need to feed my pet automatically or manually from remote places. To solve that problem I created a cloud-based IoT system that collects information from sensors and interacts with the environment using actuators.

## Architecture
The IoT device is developed using RIOT-OS and a STM NUCLEO-f401re board. The cloud-based services are based on the AWS ecosystem.
The system is composed by two sensors and two actuators:
- Ultrasonic sensor (HC SR04): measures the fill level of the dispenser. It is put on top of the food container and measures the distance from the food. While it is dispensed, the level goes down and the ultrasonic measures bigger distances.

After it has been activated, this sensor returns an echo back whose pulse width (uS) is proportional to the distance measured. This value can be divided by 58 to obtain the distance in cm. The resolution is 0.3cm and the minimum distance that can be measured is 2cm. For that reason, the sensor was placed 2cm above the maximum fill level of the food container.

When the container is empty the sensor is going to measure the distance from the screw. This distance will depend on the current rotation position of the screw, so values bigger than the threshold (5.69cm - 330uS) will be considered as empty level. The container will be considered filled when the sensor measures a distance equal to 2.59cm (150uS).
- PIR motion sensor (HC SR501): check if the pet is walking past the food dispenser. This sensor is always on. When it detects movements inside the range of a 110° angle and 5m distance, a 3.3V impulse of 3 seconds is sent to the analog pin. Every second the value emitted by the sensor is read from the board and if it is high (it has detected movements) it triggers the stepper motor.
- Stepper motor (28BYJ 48 + ULN 2003 driver): rotates a screw which dispenses the food.
- Led: lights on when the food container is empty.

The board is connected through MQTT-SN to an RSMB broker hosted on the machine the board is connected to. The connection is carried out using IPv6 and RIOT-OS tap interfaces. The board publishes on “topic_out” and subscribes to “topic_in” to receive messages from outside.
A transparent bridge written in python is used to forward messages to and from AWS IoTCore. It runs on the machine the board is connected to. It reads messages from the local broker with “topic_out” and publishes them to AWS IoTCore on the same topic. It also reads messages from AWS IoTCore with “topic_in” and publishes them on the local broker with the same topic.
In the network there will be transmitted only the fill level coming from the board and the dispense message going to the board. These messages are less than 25 bytes, so even a narrow band will be suitable for our use. Low latency is required to deliver the dispense message, as the user expects its action of clicking the button on the web dashboard to be executed in the range of 1 to 5 seconds.
The average measured latency of the system from the moment in which the ultrasonic sensor is asked to read the fill level to the point in which the result is integrated in the dashboard is less than 2 seconds. That is also because of the time the browser takes to update after it receives a message from the WebSocket.
The average measured latency of the system from the moment in which the user requests a dispense from the dashboard to the point in which the stepper motor actually dispenses food is less than 1 second.
The average round trip time from the moment in which the user requests a dispense from the dashboard to the point in which the new fill level is integrated in the dashboard is less than 5 seconds. That is because the fill level is measured after the dispenser has finished dispensing food and this process takes up to 2 seconds.
These latencies are short enough to not affect the usability of the system and are compliant with the objectives set before the development.
Data is transmitted every time the user asks to dispense food and every time food is dispensed (to store the fill level in the cloud). In the first case the message will have a fixed length of 22 bytes. In the second case the message will have a fixed length of 17 bytes. Clearly there will be overhead due to headers necessary to transmit the messages. MQTT-SN was chosen as the protocol to transmit messages because of its characteristics suitable for IoT applications, in particular for its small overhead.
Once data arrives to AWS IoTCore the computation proceeds on the AWS cloud using the following services: DynamoDB, Lambda, API Gateway, Amplify.
<img src="./media/diagram.png" width="800">
