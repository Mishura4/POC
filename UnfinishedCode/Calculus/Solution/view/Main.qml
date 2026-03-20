import QtQuick 2.15
import QtQuick.Controls 2.15
import QtCharts 2.0

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
        contentWidth: Window.width * 2
        
        ChartView {
            id: chart
            anchors.fill: parent
            antialiasing: true
            
            property double xStartMin: 0
            property double xStartMax: 0
            property double xStartRatio: 0
            property double xStartSize: 0

            ValueAxis {
                id: axisX
                min: 0
                max: 99
                titleText: "X Axis"

                Component.onCompleted: {
                    chart.xStartMin = Math.max(1, axisX.min)
                    chart.xStartMax = axisX.max
                    chart.xStartRatio = axisX.max - Math.max(1, axisX.min)
                    chart.xStartSize = window.width
                    console.log("range: ", chart.xStartSize)
                }
            }

            ValueAxis {
                id: axisY
                min: 0
                max: 10
                titleText: "Y Axis"
            }

            LineChart {
                id: lineSeries
                name: "Line"
                axisX: axisX
                axisY: axisY
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
