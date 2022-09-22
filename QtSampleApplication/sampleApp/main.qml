import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick 2.6
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.0
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import qml.components.sample 1.0

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: 640
    height: 480
    property alias comboBox: comboBox
    property int m_brightness:10
    title: qsTr("Sample Application")

    ColumnLayout{
        ComboBox {
            id: comboBox
            model: device
            textRole: "display"
            implicitWidth: 245
            height: 10
            editable: true
            MouseArea
            {
                anchors.fill: parent
                onPressed:
                {
                    if(pressed)
                    {
                        camera.callFunc()
                    }
                    mouse.accepted = false
                }
            }
            onCurrentIndexChanged:
            {
                camera.selectDevice(currentIndex)
                sideBar.getTab(0).item.children[0].enabled = true
            }
            Component.onCompleted:
            {
                camera.callFunc()
            }
        }

        TabView{
            id: sideBar
            implicitWidth: 250
            implicitHeight: 250
            Tab{
                id: uvcControls
                title:qsTr("UVC Settings");
                ColumnLayout{
                    id:tab1_combobox
                    ComboBox
                    {
                        implicitWidth: 250
                        id:format_combo_box
                        model: format
                        textRole: "display"
                        onCurrentIndexChanged:
                        {
//                            camera.enumResolution(currentIndex)
                            resolution_combo_box.enabled = true
                        }
                        Component.onCompleted: enabled = false
                    }
                    ComboBox
                    {
                        implicitWidth: 250
                        id:resolution_combo_box
                        model: resolution
                        textRole: "display"
                        onCurrentIndexChanged:
                        {
//                            camera.enumResolution(currentIndex)
                            fps_combo_box.enabled = true
                        }
                        Component.onCompleted: enabled = false
                    }
                    ComboBox
                    {
                        implicitWidth: 250
                        id:fps_combo_box
                        model: fps
                        textRole: "display"
                        onCurrentIndexChanged:
                        {
//                            camera.enumResolution(currentIndex)
                        }
                        Component.onCompleted: enabled = false
                    }
//                    GridLayout{
//                        id:gridlayout1
//                        column:3
//                        Text{
//                            id:text1
//                            text:"Brightness"
//                        }
//                        Slider{
//                            id:brightness
//                            stepSize: 1
//                            minimumValue: 0
//                            maximumValue: m_brightness
//                            onValueChanged:
//                            {
////                                camera.set_brightness(brightness.value);
//                            }
//                        }

//                    }

                }
            }
            Tab{
                id: tab2
                title: "Extention Controls"

                ColumnLayout{

                }
            }
        }

    }
    Devices{
        id: camera
    }

}


//ComboBox{
//    id: cbox1
//    implicitWidth: 200
//    height: 50

//    model: ListModel {
//        id: model1
//        ListElement { text: "See3Cam_24CUG"; color: "Yellow" }
//        ListElement { text: "See3Cam_CU135"; color: "Green" }
//        ListElement { text: "See3Cam_130"; color: "Brown" }
//    }

//}
//ComboBox{
//    id: cbox2
//    implicitWidth: 200
//    height: 10
//    model: ListModel {
//        id: model2
//        ListElement { text: "STURDeCAM20"; color: "Yellow" }
//        ListElement { text: "STURDeCAM21"; color: "Green" }
//        ListElement { text: "STURDeCAM25"; color: "Brown" }
//    }

//}
//ComboBox{
//    id: cbox3
//    implicitWidth: 200
//    height: 10
//    model: ListModel {
//        id: model3
//        ListElement { text: "NileCam21"; color: "Yellow" }
//        ListElement { text: "NileCam25"; color: "Green" }
//        ListElement { text: "NileCam81"; color: "Brown" }
//    }
//}

