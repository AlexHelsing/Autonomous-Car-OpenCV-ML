import pandas as pd
import matplotlib.pyplot as plt

# Reading the CSV data
data = pd.read_csv("steeringAngles.csv")

# Convert timestamps from microseconds to seconds for easier readability
data["Timestamp"] = (data["Timestamp"] - data["Timestamp"].min()) / 1e6

# Plotting

plt.plot(
    data["Timestamp"],
    data[" OriginalSteering"],
    label="GroundSteeringRequest.groundSteering",
    linestyle="--",
    color="red",
)
plt.plot(
    data["Timestamp"],
    data[" SteeringAngle"],
    label="Computed steering angle",
    linestyle="-",
    color="blue",
)


# Adding titles and labels
plt.title("Steering Angle Comparison Over Time")
plt.xlabel("sampleTime in microseconds")
plt.ylabel("Steering Angle")

plt.legend()


# Adding text overlay for group name
plt.text(
    0.5,
    0.5,
    "Group 16",
    fontsize=24,
    va="center",
    ha="center",
    transform=plt.gca().transAxes,
    color="black",
    alpha=1,
)

# Display the plot
plt.grid(True)
plt.show()
plt.close("all")
