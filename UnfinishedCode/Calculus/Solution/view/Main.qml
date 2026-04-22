import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphs

import com.github.Radicalware.Calculus

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("Line Chart Example")

    MarketGraph {
        id: demograph
    }

    Connections {
        target: backend
        function onNewData(time: real, value: real) {
            // lineSeries.append(time, value);
            // chart.truncate();
        }
    }

    Component.onCompleted: {
        backend.queryMarketData(new Date(), new Date());
    }
}
