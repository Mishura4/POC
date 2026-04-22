pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtGraphs

import com.github.Radicalware.Calculus

GraphsView {
    id: chart
    anchors.fill: parent
    anchors.margins: 16
    zoomStyle: GraphsView.ZoomStyle.Center

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
        gridVisible: false
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
        if (now == undefined)
            now = new Date();
        // assume NYSE for now
        now.setUTCHours(9);
        now.setUTCMinutes(0);
        return now;
    }

    readonly property var xDataMin: (dataModelMapper.model.minX ?? getOpeningTime()).getTime()
    readonly property var xDataMax: (dataModelMapper.model.maxX ?? getClosingTime()).getTime()
    property var xVisualMin: xDataMin
    property var xVisualMax: xDataMax
    property var yVisualMin: 0
    property var yVisualMax: dataModelMapper.model.maxY ?? 100

    readonly property var xVisualSpan: xVisualMin - xVisualMax
    readonly property var xVisualUnit: xVisualSpan / chart.plotArea.width

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
        xAxis.onActiveValueChanged: (delta) => {
            chart.drag(delta)
        }
    }

    function drag(delta) {
        let target = xVisualMin + delta * xVisualUnit;
        let increment;
        if (delta < 0) {
            let max = xDataMax;
            let actual = Math.min(target, max);
            increment = actual - xVisualMin;
            console.log(`max: ${max} -- target: ${target} -- actual: ${actual} -- increment: ${increment}`)
        } else {
            let min = xDataMin;
            let actual = Math.max(target, min);
            increment = actual - xVisualMin;
            console.log(`min: ${min} -- target: ${target} -- actual: ${actual} -- increment: ${increment}`)
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
        let unitDelta = -wheel.angleDelta.y * (xVisualUnit / 10);
        let leftDelta = unitDelta * leftFactor;
        let rightDelta = -1 * unitDelta * rightFactor;

        // And now, crop
        let newMin = Math.max(xDataMin, xVisualMin + leftDelta);
        let newMax = Math.min(xDataMax, xVisualMax + rightDelta);
        xVisualMin = newMin;
        xVisualMax = newMax;
    }
} // ChartView
