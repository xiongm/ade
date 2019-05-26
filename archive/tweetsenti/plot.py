import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib import style

from config import (
    SUBSCRIPTIONS,
    PLOT_STYLE,
    PLOT_X_LABEL,
    PLOT_Y_LABEL,
    CHART_WINDOW_SIZE
)

plt.ion()

class SentimentGraph:
    def __init__(self):
        self.x_data = []
        self.y_data = [[] for i in range(len(SUBSCRIPTIONS))]
        self.lines = []
        style.use(PLOT_STYLE)
        self.figure, self.ax = plt.subplots()
        for i in range(len(SUBSCRIPTIONS)):
            line, = plt.plot(self.x_data, self.y_data[i], label=SUBSCRIPTIONS[i])
            self.lines.append(line)
        plt.legend()
        plt.xlabel(PLOT_X_LABEL)
        plt.ylabel(PLOT_Y_LABEL)


    def refresh(self):
        for i in range(len(SUBSCRIPTIONS)):
            self.lines[i].set_xdata(self.x_data)
            self.lines[i].set_ydata(self.y_data[i])
        self.ax.relim()
        self.ax.autoscale_view()
        self.figure.canvas.draw()
        self.figure.canvas.flush_events()

    def update_data(self, index, senti):
        # this plot doesn't record real timestamp for x
        # since we are already tracking it in realtime
        # if we need to persist the filtered tweets, we
        # can easily add this part later
        # so the x data is just incremental sequence
        try:
            self.x_data.append(self.x_data[-1] + 1)
        except:
            self.x_data.append(1)
        if len(self.x_data) > CHART_WINDOW_SIZE:
            self.x_data.pop(0)

        # update the y data for which there's update
        self.y_data[index].append(senti)
        if len(self.y_data[index]) > CHART_WINDOW_SIZE:
            self.y_data[index].pop(0)


        # use the last value to update the rest of the plot lines
        for i in range(len(SUBSCRIPTIONS)):
            if i != index:
                data = self.y_data[i]
                try:
                    data.append(data[-1])
                except:
                    data.append(0)
                if len(data) > CHART_WINDOW_SIZE:
                    data.pop(0)
        self.refresh()
