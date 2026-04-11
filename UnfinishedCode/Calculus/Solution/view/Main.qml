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

    Flickable {
        id: flickable
        anchors.fill: parent
        contentHeight: window.height
        contentWidth: Window.width

        Rectangle {
            Button {
                text: "Refresh"

                onReleased: () => {
                    backend.queryMarketData(new Date(), new Date(), (foo) => {

                    });
                }
            }
        }

        GraphsView {
            id: chart
            anchors.fill: parent
            anchors.margins: 16

            theme: GraphsTheme {
                readonly property color c1: "#DBEB00"
                readonly property color c2: "#373F26"
                readonly property color c3: Qt.lighter(c2, 1.5)
                colorScheme: GraphsTheme.ColorScheme.Dark
                seriesColors: ["#2CDE85", "#DBEB00"]
                grid.mainColor: c3
                grid.subColor: c2
                axisX.mainColor: c3
                axisY.mainColor: c3
                axisX.subColor: c2
                axisY.subColor: c2
                axisX.labelTextColor: c1
                axisY.labelTextColor: c1
            }

            property double xStartMin: 0
            property double xStartMax: 0
            property double xStartRatio: 0
            property double xStartSize: 0
            property double xDataMin: 0
            property double xDataMax: 0
            property double yDataMin: 0
            property double yDataMax: 0

            axisX: DateTimeAxis {
                id: axisX
                titleText: "X Axis"
                labelFormat: qsTr("hh:mm:ss")
                min: dataModelMapper.model.minX ?? new Date(new Date().getTime() - 3600 * 24 * 1000)
                max: dataModelMapper.model.maxX ?? new Date()
            }

            axisY: ValueAxis {
                id: axisY
                titleText: "Y Axis"
                min: 0
                max: dataModelMapper.model.maxY ?? 100
            }

            LineSeries {
                id: lineSeries
                name: "Line"
            }

            XYModelMapper {
                id: dataModelMapper
                model: backend.model ? backend.model : []
                orientation: Qt.Vertical
                series: lineSeries
                xSection: 0
                ySection: 1

                onModelChanged: {
                    console.log("model changed");
                }

                onXSectionChanged: {
                    console.log("X changed");
                }
            }

            // Handles Zoom
            WheelHandler {
                onWheel: (wheel)=> {
                    var zoomFactor = 1.2;
                    if (wheel.modifiers & Qt.ControlModifier)
                        chart.zoom(wheel, zoomFactor)
                } // onWheel
            } // WheelHandler

            function zoom(wheel, zoomFactor) {
                if (wheel.angleDelta.y < 0) {
                    zoomFactor = 1 / zoomFactor;
                }
                var chartWidth = chart.width;
                var maxX = lineSeries.maxX();
                var axisRange = axisX.max - axisX.min;
                var centerXValue = wheel.x / chartWidth * axisRange + axisX.min;
                var newMin = centerXValue - (centerXValue - axisX.min) / zoomFactor;
                var newMax = centerXValue + (axisX.max - centerXValue) / zoomFactor;

                if (!isNaN(newMin) && !isNaN(newMax) && newMax > 1) {
                    axisX.min = Math.max(0, newMin);
                    axisX.max = Math.min(maxX, newMax);
                }

                var lnMin = Math.max(axisX.min, 1);
                var lnMax = axisX.max;
                console.log(chart.xStartSize, "<>", chart.xStartRatio, "<>", lnMin, "<>", axisX.max)
                var sizeIncrease = Math.max((chart.xStartRatio / (axisX.max - lnMin)), 1)
                //var sizeIncrease = Math.max((chart.xStartRatio / (lnMax / lnMin)), 1)
                console.log("increase:", sizeIncrease)
                flickable.contentWidth = chart.xStartSize * sizeIncrease
            }

            function truncate() {
                while (true) {
                    if (lineSeries.count == 0)
                        return;

                    var first = lineSeries.at(0).x;
                    var last = lineSeries.at(lineSeries.count - 1).x;

                    if (last - first < 10)
                        break;

                    lineSeries.remove(0)
                }
                xDataMin = first;
                xDataMax = last;
            }

            Connections {
                target: backend
                function onNewData(time: real, value: real) {
                    // lineSeries.append(time, value);
                    // chart.truncate();
                }
            }
        } // ChartView

        function adjustContentX(velocity) {
            var velocityFactor = 0.1;
            flickable.contentX += velocity * velocityFactor;
        }

        onMovementStarted: {
            adjustContentX(flickable.horizontalVelocity);
        }

        onMovementEnded: {
            adjustContentX(flickable.horizontalVelocity);
        }
    } // Flickable
}
