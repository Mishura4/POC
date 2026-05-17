import QtQuick;
import QtGraphs;

LineSeries {
    id: line

    property XYModelMapper mapper: XYModelMapper {
        orientation: Qt.Vertical
        series: line
        xSection: 0
        ySection: 1
    }
}
