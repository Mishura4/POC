pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtQuick.Shapes
import QtGraphs

import com.github.Radicalware.Calculus

Item {
    id: marketGraph
    anchors.fill: parent

    readonly property var xDataMin: (dataModelMapper.model.minX ?? getOpeningTime()).getTime()
    readonly property var xDataMax: (dataModelMapper.model.maxX ?? getClosingTime()).getTime()
    property var xVisualMin: xDataMin
    property var xVisualMax: xDataMax
    property var yVisualMin: dataModelMapper.model.minY ?? 0
    property var yVisualMax: dataModelMapper.model.maxY ?? 100

    readonly property var xVisualSpan: xVisualMax - xVisualMin
    readonly property var xVisualUnit: xVisualSpan / chart.plotArea.width

    XYModelMapper {
        id: dataModelMapper
        model: backend.model ? backend.model : []
        orientation: Qt.Vertical
        series: lineSeries
        xSection: 0
        ySection: 1
    }

    XYModelMapper {
        id: dataModelMapper2
        model: backend.model ? backend.model : []
        orientation: Qt.Vertical
        series: lineSeries2
        xSection: 2
        ySection: 3
    }

    function getOpeningTime(now) {
        if (now == undefined)
            now = new Date();
        // assume NYSE for now
        now.setUTCHours(2);
        now.setUTCMinutes(30);
        return now;
    }

    function getClosingTime(now) {
        if (now === undefined)
            now = new Date();
        // assume NYSE for now
        now.setUTCHours(9);
        now.setUTCMinutes(0);
        return now;
    }

    GraphsView {
        id: chart
        anchors.fill: parent
        anchors.margins: 16
        zoomStyle: GraphsView.ZoomStyle.Center

        theme: GraphsTheme {
            readonly property color c1: "#DBEB00"
            readonly property color c2: "#373F26"
            readonly property color c3: Qt.lighter(c2, 1.5)
            readonly property color c4: Qt.lighter(backgroundColor, 3.0)
            readonly property color c5: Qt.hsla(c4.hslHue, c4.hslSaturation, c4.hslLightness, 0.5)

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
            gridVisible: false
        }

        axisX: DateTimeAxis {
            id: axisX
            titleText: "X Axis"
            labelFormat: qsTr("hh:mm:ss")
            //timeZone: DateTimeAxis.timeZoneFromString("America/New_York")
            min: new Date(xVisualMin)
            max: new Date(xVisualMax)
        }

        axisY: ValueAxis {
            id: axisY
            titleText: "Y Axis"
            min: yVisualMin
            max: yVisualMax
            tickInterval: -1
        }

        LineSeries {
            id: lineSeries
            name: "Line"
            draggable: true
            selectable: true
            hoverable: true
        }

        LineSeries {
            id: lineSeries2
            name: "Line 2"
            draggable: true
            selectable: true
        }

        // Handles Zoom
        WheelHandler {
            onWheel: (wheel) => {
                var zoomFactor = 1.2;
                var vertical = false;
                if (wheel.modifiers & Qt.ControlModifier)
                    vertical = !vertical;
                if (wheel.modifiers & Qt.ShiftModifier)
                    zoomFactor *= 2;
                chart.zoom(wheel, zoomFactor);
            } // onWheel
        } // WheelHandler

        DragHandler {
            id: dragHandler
            target: null
            dragThreshold: 0
            yAxis.enabled: false
            xAxis.onActiveValueChanged: (delta) => {
                chart.drag(delta);
            }

            onGrabChanged: (transition, point) => {
                if (transition === PointerDevice.UngrabExclusive || transition === PointerDevice.UngrabPassive) {
                    chart.cropY();
                }
            }
        }

        Item {
            id: innerRect
            parent: chart
            x: chart.plotArea.x
            y: chart.plotArea.y
            width: chart.plotArea.width
            height: chart.plotArea.height

            HoverHandler {
                readonly property var partialPointHovered: !hoverHandler.hovered ? undefined : lineSeries.dataPointCoordinatesAt(hoverHandler.point.position.x, hoverHandler.point.position.y)
                readonly property var dataPointHovered: partialPointHovered === undefined ? undefined : dataModelMapper.model.pointClosestTo(new Date(partialPointHovered.x))
                readonly property var visualPointHovered: partialPointHovered !== undefined ? ((dataPointHovered.time.getTime() - marketGraph.xVisualMin) / (marketGraph.xVisualSpan)) * chart.plotArea.width : undefined

                id: hoverHandler
                target: Item {
                    parent: innerRect
                    id: hoverIndicator
                    anchors.fill: parent
                    visible: hoverHandler.visualPointHovered !== undefined

                    Shape {
                        ShapePath {
                            strokeColor: chart.theme.c5
                            strokeWidth: 2
                            strokeStyle: ShapePath.DashLine
                            startX: 0
                            startY: hoverHandler.point.position.y
                            PathLine { x: innerRect.width; y: hoverHandler.point.position.y }
                        }
                    }

                    Shape {
                        visible: hoverHandler.visualPointHovered >= 0 && hoverHandler.visualPointHovered <= chart.plotArea.width

                        ShapePath {
                            strokeColor: chart.theme.c5
                            strokeWidth: 2
                            strokeStyle: ShapePath.DashLine
                            startX: hoverHandler.visualPointHovered ?? 0
                            startY: 0
                            PathLine { x: hoverHandler.visualPointHovered ?? 0; y: innerRect.height }
                        }
                    }

                    Timer {
                        interval: 1000
                        running: true
                        repeat: true
                        onTriggered: {
                            console.log(`${hoverHandler.dataPointHovered}: ${marketGraph.xVisualMin} + ${marketGraph.xVisualSpan} -- ${hoverHandler.visualPointHovered}`);
                        }
                    }
                }
            }
        }

        function cropY(minX = new Date(xVisualMin), maxX = new Date(xVisualMax)) {
            let bounds = dataModelMapper.model.getBoundsY(minX, maxX);
            if (bounds === undefined || bounds === null)
                return;

            yVisualMin = bounds[0];
            yVisualMax = bounds[1];
        }

        function drag(delta) {
            let increment;
            delta *= -1;
            if (delta < 0) {
                let target = xVisualMin + delta * xVisualUnit;
                let min = xDataMin;
                let actual = Math.max(target, min);
                increment = actual - xVisualMin;
            } else {
                let target = xVisualMax + delta * xVisualUnit;
                let max = xDataMax;
                let actual = Math.min(target, max);
                increment = actual - xVisualMax;
            }
            xVisualMin += increment;
            xVisualMax += increment;
        }

        function zoom(wheel, zoomFactor) {
            // Get relative position on graph
            const plot = chart.plotArea;
            let relX = wheel.x - plot.x;
            if (relX < 0 || relX >= plot.width)
                return; // We're outside

            let leftFactor = (relX / plot.width);
            let rightFactor = (plot.width - relX) / plot.width;

            // Zoom is relative to the window we're viewing
            let unitDelta = wheel.angleDelta.y * (xVisualUnit / 10);
            let leftDelta = unitDelta * leftFactor;
            let rightDelta = -1 * unitDelta * rightFactor;

            // And now, crop
            let newMin = Math.max(xDataMin, xVisualMin + leftDelta);
            let newMax = Math.min(xDataMax, xVisualMax + rightDelta);
            xVisualMin = newMin;
            xVisualMax = newMax;
            cropY();
        }
    } // ChartView
}
