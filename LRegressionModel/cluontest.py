from pycluon.importer import import_odvd
from pycluon import OD4Session, Envelope
import time
import joblib

import pandas as pd
import time


from pycluon import SharedMemory

# model = joblib.load("./Models/AngularOnly/ao_model.pkl")
# scaler = joblib.load("./Models/AngularOnly/ao_scaler.pkl")
# imputer = joblib.load("./Models/AngularOnly/ao_imputer.pkl")

# model = joblib.load("./Models/RFT_ANGULAR/ao_model.pkl")
# scaler = joblib.load("./Models/RFT_ANGULAR/ao_scaler.pkl")
# imputer = joblib.load("./Models/RFT_ANGULAR/ao_imputer.pkl")

# model = joblib.load("./ao_model.pkl")
# scaler = joblib.load("./ao_scaler.pkl")
# imputer = joblib.load("./ao_imputer.pkl")

model = joblib.load("./Models/RFT_GRIDSEARCH/ao_model.pkl")
scaler = joblib.load("./Models/RFT_GRIDSEARCH/ao_scaler.pkl")
imputer = joblib.load("./Models/RFT_GRIDSEARCH/ao_imputer.pkl")


sm = SharedMemory("img")

my_odvd = import_odvd("gg.odvd")

session = OD4Session(253)


if not session.is_running():
    print("Session is not running. Check network or CID issues.")
else:
    print("Session is running.")


def callback(envelope):
    print("Received")
    print("Received message from CID 1031:" + str(envelope.serialized_data))
    print(my_odvd)
    try:
        message = my_odvd.AngularVelocityReading()
        message.ParseFromString(envelope.serialized_data)
        print(message)
        print("Message parsed successfully.")
    except Exception as e:
        print(f"Failed to parse message: {e}")


def angular_velocity_callback(envelope):
    print("Received message with serialized data:", envelope.serialized_data)
    angular_velocity = (
        my_odvd.AngularVelocityReading()
    )  # Access the class from the dynamically imported module
    angular_velocity.ParseFromString(envelope.serialized_data)  # Deserialize the data

    data = {
        "angularVelocityX": angular_velocity.angularVelocityX,
        "angularVelocityY": angular_velocity.angularVelocityY,
        "angularVelocityZ": angular_velocity.angularVelocityZ,
    }

    df = pd.DataFrame([data])
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


# Assuming you have a session instance named `session`
session.add_data_trigger(1031, angular_velocity_callback)


while session.is_running():
    sm.wait()
    sm.lock()
    print("Shared Memory Timestamp:", sm.timestamp)
    sm.unlock()

    time.sleep(1)
