#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libfacemanagement_SRCS
    faceutils.cpp
    facegroup.cpp
    facepipeline.cpp
    facepipeline_p.cpp
    facebenchmarkers.cpp
    faceworkers.cpp
    faceitemretriever.cpp
    facepreviewloader.cpp
    facescandialog.cpp
    assignnamewidget.cpp
    parallelpipes.cpp
    scanstatefilter.cpp
)

include_directories($<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt5::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

add_library(facemanagement_src OBJECT ${libfacemanagement_SRCS})
