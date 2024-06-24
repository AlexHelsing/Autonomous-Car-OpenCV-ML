import pandas as pd
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LinearRegression
from sklearn.ensemble import RandomForestRegressor
from sklearn.model_selection import train_test_split, GridSearchCV
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_squared_error
from sklearn.impute import SimpleImputer
import joblib


def load_and_clean(filepath, columns):
    """Load CSV file with specified delimiter and keep only necessary columns."""
    df = pd.read_csv(filepath, delimiter=";", usecols=columns)

    df["timestamp"] = pd.to_datetime(
        df["sampleTimeStamp.seconds"] * 1e6 + df["sampleTimeStamp.microseconds"],
        unit="us",
    )

    # drop og timestamp cols
    df.drop(
        ["sampleTimeStamp.seconds", "sampleTimeStamp.microseconds"],
        axis=1,
        inplace=True,
    )

    df["timestamp"] = df["timestamp"].dt.round("0.5s")

    return df


def merge_dataframes(dataframes, merge_on):
    """Merge dataframes based on the given keys."""
    merged_df = dataframes[0]
    for df in dataframes[1:]:
        merged_df = pd.merge(merged_df, df, on=merge_on, how="inner")

    return merged_df


def train_model(combined_df):
    """Train a Random Forest model."""
    # Normalize data for training/model preparation
    scaler = StandardScaler()
    features_to_scale = [
        "angularVelocityX",
        "angularVelocityY",
        "angularVelocityZ",
        "voltage_x",
        "voltage_y",
    ]

    print(combined_df[features_to_scale])

    combined_df[features_to_scale] = scaler.fit_transform(
        combined_df[features_to_scale]
    )

    # Separate features and target variable
    X = combined_df[features_to_scale]
    y = combined_df["groundSteering"]

    # Handle missing data in features
    imputer = SimpleImputer(strategy="mean")
    X = imputer.fit_transform(X)

    # Use a subset of data to speed up the grid search
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.8, random_state=42
    )  # Reducing training data size

    # Simplified grid
    param_grid = {
        "n_estimators": [50, 100],  # Reduced complexity
        "max_depth": [10, None],  # Reduced options
        "min_samples_leaf": [3, 4],  # Fewer options
        "max_features": ["sqrt"],  # Fixed to one to reduce computation
    }

    grid_search = GridSearchCV(
        estimator=RandomForestRegressor(random_state=42),
        param_grid=param_grid,
        cv=3,
        scoring="neg_mean_squared_error",
        n_jobs=-1,
        verbose=2,
    )
    grid_search.fit(X_train, y_train)

    # Get the best estimator
    model = grid_search.best_estimator_

    # You can adjust these hyperparameters
    model.fit(X_train, y_train)

    # Evaluate the model
    predictions = model.predict(X_test)
    mse = mean_squared_error(y_test, predictions)
    print(f"Mean squared error: {mse}")

    return model, imputer, scaler


# data[0] = "CSV-Files/REC0 /opendlv.proxy.VoltageReading-1.csv"
# data[1] = "CSV-Files/REC0 /opendlv.proxy.VoltageReading-3.csv"
# data[2] = "CSV-Files/REC0 /opendlv.proxy.GroundSteeringRequest-0.csv"
# data[3] = "CSV-Files/REC0 /opendlv.proxy.AngularVelocityReading-0.csv"
def preprocess_data(
    voltageReading1,
    voltageReading3,
    GroundSteering,
    AngularVelocityReading,
    for_training=True,
):
    columns_IR = ["sampleTimeStamp.seconds", "sampleTimeStamp.microseconds", "voltage"]
    columns_velocity = [
        "sampleTimeStamp.seconds",
        "sampleTimeStamp.microseconds",
        "angularVelocityX",
        "angularVelocityY",
        "angularVelocityZ",
    ]

    merge_on = ["timestamp"]

    # Steering is only available during training
    if for_training:

        columns_steering = [
            "sampleTimeStamp.seconds",
            "sampleTimeStamp.microseconds",
            "groundSteering",
        ]
        Steering = load_and_clean(GroundSteering, columns_steering)
        Velocity = load_and_clean(AngularVelocityReading, columns_velocity)
        IR_Left = load_and_clean(voltageReading1, columns_IR)
        IR_Right = load_and_clean(voltageReading3, columns_IR)
        # Merge dataframes based on timestamp since dataflow is not perfectly synchronized

        combined_df = merge_dataframes(
            [Steering, Velocity, IR_Left, IR_Right], merge_on
        )

        # print the head of the data
        print(combined_df.head())
    else:
        Velocity = load_and_clean(AngularVelocityReading, columns_velocity)

        combined_df = Velocity

        # drop og timestamp cols
        combined_df.drop(
            ["sampleTimeStamp.seconds", "sampleTimeStamp.microseconds"],
            axis=1,
            inplace=True,
        )

    # Sort dataframe by timestamp if necessary
    combined_df.sort_values(by=merge_on, inplace=True)

    # save the combined data
    combined_df.to_csv("combined_data.csv")

    return combined_df


def preprocess_prediction_data(filepath, columns, scaler, imputer):
    """Load and preprocess new data for making predictions."""

    # load the data, wwe dont need to use the clean function since the data is already cleaned
    Velocity = pd.read_csv(filepath[0], delimiter=";", usecols=columns)

    Velocity = Velocity.rename(
        columns={
            "sampleTimeStamp.seconds": "sampleTimeStamp.seconds",
            "sampleTimeStamp.microseconds": "sampleTimeStamp.microseconds",
            "angularVelocityX": "angularVelocityX",
            "angularVelocityY": "angularVelocityY",
            "angularVelocityZ": "angularVelocityZ",
        }
    )
    # print the head of the data
    print(Velocity.head())

    #
    # Normalize data for training/model preparation
    features_to_scale = [
        "angularVelocityX",
        "angularVelocityY",
        "angularVelocityZ",
    ]

    Velocity[features_to_scale] = scaler.transform(Velocity[features_to_scale])

    # handle missing data in features
    Velocity = imputer.transform(Velocity)

    return Velocity


def test_model(model, new_data):
    """Make predictions using the trained model."""

    predictions = model.predict(new_data)

    return predictions


def loadAllData(file_groups):
    all_data_frames = []
    for group in file_groups:
        preprocessed_data = preprocess_data(
            voltageReading1=group[0],
            voltageReading3=group[1],
            GroundSteering=group[2],
            AngularVelocityReading=group[3],
            for_training=True,
        )
        all_data_frames.append(preprocessed_data)

    return pd.concat(all_data_frames, ignore_index=True)


def smooth_predictions(predictions, window_size=5):
    """Apply a simple moving average to the predictions."""
    predictions_df = pd.DataFrame(predictions, columns=["predicted_steering"])
    predictions_df["smoothed"] = (
        predictions_df["predicted_steering"]
        .rolling(window=window_size, min_periods=1)
        .mean()
    )
    return predictions_df


if __name__ == "__main__":

    RecordingToPredict = [
        "CSV-Files/CID-140-recording-2020-03-18_150001-selection.rec.csv/opendlv.proxy.AngularVelocityReading-0.csv",
    ]

    # TrainingData = preprocess_data(
    #     voltageReading1="CSV-Files/REC0/opendlv.proxy.VoltageReading-1.csv",
    #     voltageReading3="CSV-Files/REC0/opendlv.proxy.VoltageReading-3.csv",
    #     GroundSteering="CSV-Files/REC0/opendlv.proxy.GroundSteeringRequest-0.csv",
    #     AngularVelocityReading="CSV-Files/REC0/opendlv.proxy.AngularVelocityReading-0.csv",
    #     for_training=True,
    # )

    file_paths = [
        [
            "CSV-Files/CID-140-recording-2020-03-18_144821-selection.rec.csv/opendlv.proxy.VoltageReading-1.csv",
            "CSV-Files/CID-140-recording-2020-03-18_144821-selection.rec.csv/opendlv.proxy.VoltageReading-3.csv",
            "CSV-Files/CID-140-recording-2020-03-18_144821-selection.rec.csv/opendlv.proxy.GroundSteeringRequest-0.csv",
            "CSV-Files/CID-140-recording-2020-03-18_144821-selection.rec.csv/opendlv.proxy.AngularVelocityReading-0.csv",
        ],
        [
            "CSV-Files/CID-140-recording-2020-03-18_145043-selection.rec.csv/opendlv.proxy.VoltageReading-1.csv",
            "CSV-Files/CID-140-recording-2020-03-18_145043-selection.rec.csv/opendlv.proxy.VoltageReading-3.csv",
            "CSV-Files/CID-140-recording-2020-03-18_145043-selection.rec.csv/opendlv.proxy.GroundSteeringRequest-0.csv",
            "CSV-Files/CID-140-recording-2020-03-18_145043-selection.rec.csv/opendlv.proxy.AngularVelocityReading-0.csv",
        ],
        [
            "CSV-Files/CID-140-recording-2020-03-18_145233-selection.rec.csv/opendlv.proxy.VoltageReading-1.csv",
            "CSV-Files/CID-140-recording-2020-03-18_145233-selection.rec.csv/opendlv.proxy.VoltageReading-3.csv",
            "CSV-Files/CID-140-recording-2020-03-18_145233-selection.rec.csv/opendlv.proxy.GroundSteeringRequest-0.csv",
            "CSV-Files/CID-140-recording-2020-03-18_145233-selection.rec.csv/opendlv.proxy.AngularVelocityReading-0.csv",
        ],
        [
            "CSV-Files/CID-140-recording-2020-03-18_145641-selection.rec.csv/opendlv.proxy.VoltageReading-1.csv",
            "CSV-Files/CID-140-recording-2020-03-18_145641-selection.rec.csv/opendlv.proxy.VoltageReading-3.csv",
            "CSV-Files/CID-140-recording-2020-03-18_145641-selection.rec.csv/opendlv.proxy.GroundSteeringRequest-0.csv",
            "CSV-Files/CID-140-recording-2020-03-18_145641-selection.rec.csv/opendlv.proxy.AngularVelocityReading-0.csv",
        ],
        [
            "CSV-Files/CID-140-recording-2020-03-18_150001-selection.rec.csv/opendlv.proxy.VoltageReading-1.csv",
            "CSV-Files/CID-140-recording-2020-03-18_150001-selection.rec.csv/opendlv.proxy.VoltageReading-3.csv",
            "CSV-Files/CID-140-recording-2020-03-18_150001-selection.rec.csv/opendlv.proxy.GroundSteeringRequest-0.csv",
            "CSV-Files/CID-140-recording-2020-03-18_150001-selection.rec.csv/opendlv.proxy.AngularVelocityReading-0.csv",
        ],
    ]

    # # Load all data
    TrainingData = loadAllData(file_paths)

    # Train model
    [model, imputer, scaler] = train_model(TrainingData)

    # Save model
    joblib.dump(model, "ao_model2.pkl")
    # Save imputer
    joblib.dump(imputer, "ao_imputer2.pkl")
    # Save scaler
    joblib.dump(scaler, "ao_scaler2.pkl")

    # use the model to predict the steering angle using some arbitrary data, one
    PredictionData = preprocess_prediction_data(
        RecordingToPredict,
        columns=[
            "angularVelocityX",
            "angularVelocityY",
            "angularVelocityZ",
            "voltage_x",
            "voltage_y",
        ],
        scaler=scaler,
        imputer=imputer,
    )
