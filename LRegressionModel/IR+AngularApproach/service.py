from pycluon.importer import import_odvd
from pycluon import OD4Session, Envelope
import time
import joblib
import numpy as np
import pandas as pd
import time


from pycluon import SharedMemory

# model = joblib.load("./Models/AngularOnly/ao_model.pkl")
# scaler = joblib.load("./Models/AngularOnly/ao_scaler.pkl")
# imputer = joblib.load("./Models/AngularOnly/ao_imputer.pkl")

# model = joblib.load("./Models/RFT_ANGULAR/ao_model.pkl")
# scaler = joblib.load("./Models/RFT_ANGULAR/ao_scaler.pkl")
# imputer = joblib.load("./Models/RFT_ANGULAR/ao_imputer.pkl")

model = joblib.load("./ao_model2.pkl")
scaler = joblib.load("./ao_scaler2.pkl")
imputer = joblib.load("./ao_imputer2.pkl")


sm = SharedMemory("img")

my_odvd = import_odvd("gg.odvd")

session = OD4Session(253)


if not session.is_running():
    print("Session is not running. Check network or CID issues.")
else:
    print("Session is running.")


# initalize a dict to store sensor data
sensor_data = {
    "angularVelocityX": None,
    "angularVelocityY": None,
    "angularVelocityZ": None,
    "IR_Left": None,
    "IR_Right": None,
}


def update_and_predict():
    print("Sensor data:", sensor_data)
    if all(value is not None for value in sensor_data.values()):
        df = pd.DataFrame([sensor_data])

        # change name of IR_Left to voltage_x and IR_Right to voltage_y
        df = df.rename(columns={"IR_Left": "voltage_x", "IR_Right": "voltage_y"})

        # print first index of df
        print(df.iloc[0])

        df_scaled = scaler.transform(df)
        df_imputed = imputer.transform(df_scaled)

        # Predict
        prediction = model.predict(df_imputed)
        print("Predicted steering:", prediction)

        steering_cmd = my_odvd.SteeringCommand()
        steering_cmd.steeringAngle = prediction[0]

        envelopeToSend = Envelope()
        envelopeToSend.serialized_data = steering_cmd.SerializeToString()
        envelopeToSend.data_type = 1234
        envelopeToSend.sender_stamp = 1234

        session.send(envelopeToSend)

        # Reset sensor data
        for key in sensor_data:
            sensor_data[key] = None

        print("Sent steering command")


def angular_velocity_callback(envelope):

    angular_velocity = (
        my_odvd.AngularVelocityReading()
    )  # Access the class from the dynamically imported module
    angular_velocity.ParseFromString(envelope.serialized_data)  # Deserialize the data

    sensor_data["angularVelocityX"] = angular_velocity.angularVelocityX
    sensor_data["angularVelocityY"] = angular_velocity.angularVelocityY
    sensor_data["angularVelocityZ"] = angular_velocity.angularVelocityZ

    update_and_predict()


def IR_callback(envelope):

    # check sender stamp

    # if sender stamp is 1 => it is left IR sensor
    if envelope.sender_stamp == 1:
        print("Left IR Sensor")
        IR_Left = my_odvd.VoltageReading()
        IR_Left.ParseFromString(envelope.serialized_data)

        sensor_data["IR_Left"] = IR_Left.voltage

        update_and_predict()

    # if sender stamp is 2 => it is right IR sensor
    elif envelope.sender_stamp == 3:
        print("Right IR Sensor")
        IR_Right = my_odvd.VoltageReading()
        IR_Right.ParseFromString(envelope.serialized_data)

        print(IR_Right)

        sensor_data["IR_Right"] = IR_Right.voltage

        update_and_predict()


def callback(envelope):

    print(my_odvd)
    try:
        message = my_odvd.AngularVelocityReading()
        message.ParseFromString(envelope.serialized_data)
        print(message)
        print("Message parsed successfully.")
    except Exception as e:
        print(f"Failed to parse message: {e}")


# def angular_velocity_callback(envelope):
#     print("Received message with serialized data:", envelope.serialized_data)
#     angular_velocity = (
#         my_odvd.AngularVelocityReading()
#     )  # Access the class from the dynamically imported module
#     angular_velocity.ParseFromString(envelope.serialized_data)  # Deserialize the data

#     data = {
#         "angularVelocityX": angular_velocity.angularVelocityX,
#         "angularVelocityY": angular_velocity.angularVelocityY,
#         "angularVelocityZ": angular_velocity.angularVelocityZ,
#     }

#     df = pd.DataFrame([data])
#     df_scaled = scaler.transform(df)
#     df_imputed = imputer.transform(df_scaled)

#     # Predict
#     prediction = model.predict(df_imputed)
#     print("Predicted steering:", prediction)

#     steering_cmd = my_odvd.SteeringCommand()
#     steering_cmd.steeringAngle = prediction[0]

#     envelopeToSend = Envelope()
#     envelopeToSend.serialized_data = steering_cmd.SerializeToString()
#     envelopeToSend.data_type = 1234
#     envelopeToSend.sender_stamp = 1234

#     session.send(envelopeToSend)


# Assuming you have a session instance named `session`
session.add_data_trigger(1031, angular_velocity_callback)
session.add_data_trigger(1037, IR_callback)


while session.is_running():
    sm.wait()
    sm.lock()
    print("Shared Memory Timestamp:", sm.timestamp)
    sm.unlock()

    time.sleep(1)
