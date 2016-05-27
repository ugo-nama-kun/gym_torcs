@echo off

set RUNTIMEDIR=%1
if defined RUNTIMEDIR goto build
echo Parameter missing, please do not execute generic scripts directly
exit /b

:build

call .\create_dir .\export
call .\create_dir .\export\lib
call .\create_dir .\export\libd

if exist .\src\interfaces\*.* call .\create_dir .\export
if exist .\src\interfaces\*.* call .\create_dir .\export\include
if exist .\src\interfaces\car.h copy .\src\interfaces\car.h .\export\include\car.h
if exist .\src\interfaces\graphic.h copy .\src\interfaces\graphic.h .\export\include\graphic.h
if exist .\src\interfaces\js.h copy .\src\interfaces\js.h .\export\include\js.h
if exist .\src\interfaces\playerpref.h copy .\src\interfaces\playerpref.h .\export\include\playerpref.h
if exist .\src\interfaces\raceman.h copy .\src\interfaces\raceman.h .\export\include\raceman.h
if exist .\src\interfaces\replay.h copy .\src\interfaces\replay.h .\export\include\replay.h
if exist .\src\interfaces\robot.h copy .\src\interfaces\robot.h .\export\include\robot.h
if exist .\src\interfaces\simu.h copy .\src\interfaces\simu.h .\export\include\simu.h
if exist .\src\interfaces\telemetry.h copy .\src\interfaces\telemetry.h .\export\include\telemetry.h
if exist .\src\interfaces\track.h copy .\src\interfaces\track.h .\export\include\track.h

if exist .\src\libs\txml\*.* call .\create_dir .\export
if exist .\src\libs\txml\*.* call .\create_dir .\export\include
if exist .\src\libs\txml\xml.h copy .\src\libs\txml\xml.h .\export\include\xml.h
if exist .\src\libs\txml\xmlparse.h copy .\src\libs\txml\xmlparse.h .\export\include\xmlparse.h

if exist .\src\libs\tgf\*.* call .\create_dir .\export
if exist .\src\libs\tgf\*.* call .\create_dir .\export\include
if exist .\src\libs\tgf\tgf.h copy .\src\libs\tgf\tgf.h .\export\include\tgf.h
if exist .\src\libs\tgf\os.h copy .\src\libs\tgf\os.h .\export\include\os.h

if exist .\src\libs\tgfclient\*.* call .\create_dir .\export
if exist .\src\libs\tgfclient\*.* call .\create_dir .\export\include
if exist .\src\libs\tgfclient\tgfclient.h copy .\src\libs\tgfclient\tgfclient.h .\export\include\tgfclient.h
if exist .\src\libs\tgfclient\screen_properties.h copy .\src\libs\tgfclient\screen_properties.h .\export\include\screen_properties.h
if exist .\src\libs\tgfclient\glfeatures.h copy .\src\libs\tgfclient\glfeatures.h .\export\include\glfeatures.h

if exist .\src\libs\client\*.* call .\create_dir .\export
if exist .\src\libs\client\*.* call .\create_dir .\export\include
if exist .\src\libs\client\client.h copy .\src\libs\client\client.h .\export\include\client.h
if exist .\src\libs\client\exitmenu.h copy .\src\libs\client\exitmenu.h .\export\include\exitmenu.h

if exist .\src\libs\confscreens\*.* call .\create_dir .\export
if exist .\src\libs\confscreens\*.* call .\create_dir .\export\include
if exist .\src\libs\confscreens\confscreens.h copy .\src\libs\confscreens\confscreens.h .\export\include\confscreens.h
if exist .\src\libs\confscreens\driverconfig.h copy .\src\libs\confscreens\driverconfig.h .\export\include\driverconfig.h
if exist .\src\libs\confscreens\joystickconfig.h copy .\src\libs\confscreens\joystickconfig.h .\export\include\joystickconfig.h
if exist .\src\libs\confscreens\mouseconfig.h copy .\src\libs\confscreens\mouseconfig.h .\export\include\mouseconfig.h
if exist .\src\libs\confscreens\controlconfig.h copy .\src\libs\confscreens\controlconfig.h .\export\include\controlconfig.h
if exist .\src\libs\confscreens\graphconfig.h copy .\src\libs\confscreens\graphconfig.h .\export\include\graphconfig.h
if exist .\src\libs\confscreens\soundconfig.h copy .\src\libs\confscreens\soundconfig.h .\export\include\soundconfig.h
if exist .\src\libs\confscreens\simuconfig.h copy .\src\libs\confscreens\simuconfig.h .\export\include\simuconfig.h
if exist .\src\libs\confscreens\openglconfig.h copy .\src\libs\confscreens\openglconfig.h .\export\include\openglconfig.h

if exist .\src\libs\racescreens\*.* call .\create_dir .\export
if exist .\src\libs\racescreens\*.* call .\create_dir .\export\include
if exist .\src\libs\racescreens\racescreens.h copy .\src\libs\racescreens\racescreens.h .\export\include\racescreens.h

if exist .\src\libs\robottools\*.* call .\create_dir .\export
if exist .\src\libs\robottools\*.* call .\create_dir .\export\include
if exist .\src\libs\robottools\robottools.h copy .\src\libs\robottools\robottools.h .\export\include\robottools.h

if exist .\src\libs\raceengineclient\*.* call .\create_dir .\export
if exist .\src\libs\raceengineclient\*.* call .\create_dir .\export\include
if exist .\src\libs\raceengineclient\singleplayer.h copy .\src\libs\raceengineclient\singleplayer.h .\export\include\singleplayer.h
if exist .\src\libs\raceengineclient\raceinit.h copy .\src\libs\raceengineclient\raceinit.h .\export\include\raceinit.h

if exist .\src\linux\*.* call .\create_dir .\export
if exist .\src\linux\*.* call .\create_dir .\export\include
if exist .\src\windows\osspec.h copy .\src\windows\osspec.h .\export\include\osspec.h

if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\*.* call .\create_dir .\export
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\*.* call .\create_dir .\export\include
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\*.* call .\create_dir .\export\include\3D
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Basic.h copy .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Basic.h .\export\include\3D\Basic.h
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Matrix.h copy .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Matrix.h .\export\include\3D\Matrix.h
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Point.h copy .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Point.h .\export\include\3D\Point.h
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Quaternion.h copy .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Quaternion.h .\export\include\3D\Quaternion.h
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Tuple3.h copy .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Tuple3.h .\export\include\3D\Tuple3.h
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Tuple4.h copy .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Tuple4.h .\export\include\3D\Tuple4.h
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Vector.h copy .\src\modules\simu\simuv2\SOLID-2.0\include\3D\Vector.h .\export\include\3D\Vector.h

if exist .\src\modules\simu\simuv2\SOLID-2.0\include\SOLID\*.* call .\create_dir .\export
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\SOLID\*.* call .\create_dir .\export\include
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\SOLID\*.* call .\create_dir .\export\include\SOLID
if exist .\src\modules\simu\simuv2\SOLID-2.0\include\SOLID\solid.h copy .\src\modules\simu\simuv2\SOLID-2.0\include\SOLID\solid.h .\export\include\SOLID\solid.h

if exist .\src\libs\math\*.* call .\create_dir .\export
if exist .\src\libs\math\*.* call .\create_dir .\export\include
if exist .\src\libs\math\*.* call .\create_dir .\export\include\tmath
if exist .\src\libs\math\linalg_t.h copy .\src\libs\math\linalg_t.h .\export\include\tmath\linalg_t.h
if exist .\src\libs\math\straight2_t.h copy .\src\libs\math\straight2_t.h .\export\include\tmath\straight2_t.h
if exist .\src\libs\math\v4_t.h copy .\src\libs\math\v4_t.h .\export\include\tmath\v4_t.h
if exist .\src\libs\math\v3_t.h copy .\src\libs\math\v3_t.h .\export\include\tmath\v3_t.h
if exist .\src\libs\math\v2_t.h copy .\src\libs\math\v2_t.h .\export\include\tmath\v2_t.h

if exist .\src\libs\learning\*.* call .\create_dir .\export
if exist .\src\libs\learning\*.* call .\create_dir .\export\include
if exist .\src\libs\learning\*.* call .\create_dir .\export\include\learning
if exist .\src\libs\learning\policy.h copy .\src\libs\learning\policy.h .\export\include\learning\policy.h
if exist .\src\libs\learning\ann_policy.h copy .\src\libs\learning\ann_policy.h .\export\include\learning\ann_policy.h
if exist .\src\libs\learning\ANN.h copy .\src\libs\learning\ANN.h .\export\include\learning\ANN.h
if exist .\src\libs\learning\learn_debug.h copy .\src\libs\learning\learn_debug.h .\export\include\learning\learn_debug.h
if exist .\src\libs\learning\real.h copy .\src\libs\learning\real.h .\export\include\learning\real.h
if exist .\src\libs\learning\string_utils.h copy .\src\libs\learning\string_utils.h .\export\include\learning\string_utils.h
if exist .\src\libs\learning\List.h copy .\src\libs\learning\List.h .\export\include\learning\List.h
if exist .\src\libs\learning\MathFunctions.h copy .\src\libs\learning\MathFunctions.h .\export\include\learning\MathFunctions.h
if exist .\src\libs\learning\Distribution.h copy .\src\libs\learning\Distribution.h .\export\include\learning\Distribution.h
rem if exist .\src\libs\learning\SmartAssert.h copy .\src\libs\learning\SmartAssert.h .\export\include\learning\SmartAssert.h

if exist .\src\libs\portability\*.* call .\create_dir .\export
if exist .\src\libs\portability\*.* call .\create_dir .\export\include
if exist .\src\libs\portability\portability.h copy .\src\libs\portability\portability.h .\export\include\portability.h

if exist .\src\drivers\berniw\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\1
if exist .\src\drivers\berniw\1\cg-nascar-rwd.rgb copy .\src\drivers\berniw\1\cg-nascar-rwd.rgb .\%RUNTIMEDIR%\drivers\berniw\1\cg-nascar-rwd.rgb
if exist .\src\drivers\berniw\1\aalborg.xml copy .\src\drivers\berniw\1\aalborg.xml .\%RUNTIMEDIR%\drivers\berniw\1\aalborg.xml
if exist .\src\drivers\berniw\1\alpine-1.xml copy .\src\drivers\berniw\1\alpine-1.xml .\%RUNTIMEDIR%\drivers\berniw\1\alpine-1.xml
if exist .\src\drivers\berniw\1\a-speedway.xml copy .\src\drivers\berniw\1\a-speedway.xml .\%RUNTIMEDIR%\drivers\berniw\1\a-speedway.xml
if exist .\src\drivers\berniw\1\default.xml copy .\src\drivers\berniw\1\default.xml .\%RUNTIMEDIR%\drivers\berniw\1\default.xml
if exist .\src\drivers\berniw\1\dirt-1.xml copy .\src\drivers\berniw\1\dirt-1.xml .\%RUNTIMEDIR%\drivers\berniw\1\dirt-1.xml
if exist .\src\drivers\berniw\1\dirt-2.xml copy .\src\drivers\berniw\1\dirt-2.xml .\%RUNTIMEDIR%\drivers\berniw\1\dirt-2.xml
if exist .\src\drivers\berniw\1\dirt-3.xml copy .\src\drivers\berniw\1\dirt-3.xml .\%RUNTIMEDIR%\drivers\berniw\1\dirt-3.xml
if exist .\src\drivers\berniw\1\dirt-4.xml copy .\src\drivers\berniw\1\dirt-4.xml .\%RUNTIMEDIR%\drivers\berniw\1\dirt-4.xml
if exist .\src\drivers\berniw\1\dirt-5.xml copy .\src\drivers\berniw\1\dirt-5.xml .\%RUNTIMEDIR%\drivers\berniw\1\dirt-5.xml
if exist .\src\drivers\berniw\1\dirt-6.xml copy .\src\drivers\berniw\1\dirt-6.xml .\%RUNTIMEDIR%\drivers\berniw\1\dirt-6.xml
if exist .\src\drivers\berniw\1\eroad.xml copy .\src\drivers\berniw\1\eroad.xml .\%RUNTIMEDIR%\drivers\berniw\1\eroad.xml
if exist .\src\drivers\berniw\1\e-track-1.xml copy .\src\drivers\berniw\1\e-track-1.xml .\%RUNTIMEDIR%\drivers\berniw\1\e-track-1.xml
if exist .\src\drivers\berniw\1\e-track-2.xml copy .\src\drivers\berniw\1\e-track-2.xml .\%RUNTIMEDIR%\drivers\berniw\1\e-track-2.xml
if exist .\src\drivers\berniw\1\e-track-3.xml copy .\src\drivers\berniw\1\e-track-3.xml .\%RUNTIMEDIR%\drivers\berniw\1\e-track-3.xml
if exist .\src\drivers\berniw\1\e-track-4.xml copy .\src\drivers\berniw\1\e-track-4.xml .\%RUNTIMEDIR%\drivers\berniw\1\e-track-4.xml
if exist .\src\drivers\berniw\1\e-track-5.xml copy .\src\drivers\berniw\1\e-track-5.xml .\%RUNTIMEDIR%\drivers\berniw\1\e-track-5.xml
if exist .\src\drivers\berniw\1\e-track-6.xml copy .\src\drivers\berniw\1\e-track-6.xml .\%RUNTIMEDIR%\drivers\berniw\1\e-track-6.xml
if exist .\src\drivers\berniw\1\g-track-1.xml copy .\src\drivers\berniw\1\g-track-1.xml .\%RUNTIMEDIR%\drivers\berniw\1\g-track-1.xml
if exist .\src\drivers\berniw\1\g-track-2.xml copy .\src\drivers\berniw\1\g-track-2.xml .\%RUNTIMEDIR%\drivers\berniw\1\g-track-2.xml
if exist .\src\drivers\berniw\1\g-track-3.xml copy .\src\drivers\berniw\1\g-track-3.xml .\%RUNTIMEDIR%\drivers\berniw\1\g-track-3.xml
if exist .\src\drivers\berniw\1\michigan.xml copy .\src\drivers\berniw\1\michigan.xml .\%RUNTIMEDIR%\drivers\berniw\1\michigan.xml
if exist .\src\drivers\berniw\1\mixed-1.xml copy .\src\drivers\berniw\1\mixed-1.xml .\%RUNTIMEDIR%\drivers\berniw\1\mixed-1.xml
if exist .\src\drivers\berniw\1\mixed-2.xml copy .\src\drivers\berniw\1\mixed-2.xml .\%RUNTIMEDIR%\drivers\berniw\1\mixed-2.xml
if exist .\src\drivers\berniw\1\spring.xml copy .\src\drivers\berniw\1\spring.xml .\%RUNTIMEDIR%\drivers\berniw\1\spring.xml
if exist .\src\drivers\berniw\1\wheel-1.xml copy .\src\drivers\berniw\1\wheel-1.xml .\%RUNTIMEDIR%\drivers\berniw\1\wheel-1.xml

if exist .\src\drivers\berniw\10\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\10
if exist .\src\drivers\berniw\10\car1-trb3.rgb copy .\src\drivers\berniw\10\car1-trb3.rgb .\%RUNTIMEDIR%\drivers\berniw\10\car1-trb3.rgb
if exist .\src\drivers\berniw\10\default.xml copy .\src\drivers\berniw\10\default.xml .\%RUNTIMEDIR%\drivers\berniw\10\default.xml

if exist .\src\drivers\berniw\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\2
if exist .\src\drivers\berniw\2\cg-nascar-rwd.rgb copy .\src\drivers\berniw\2\cg-nascar-rwd.rgb .\%RUNTIMEDIR%\drivers\berniw\2\cg-nascar-rwd.rgb
if exist .\src\drivers\berniw\2\aalborg.xml copy .\src\drivers\berniw\2\aalborg.xml .\%RUNTIMEDIR%\drivers\berniw\2\aalborg.xml
if exist .\src\drivers\berniw\2\alpine-1.xml copy .\src\drivers\berniw\2\alpine-1.xml .\%RUNTIMEDIR%\drivers\berniw\2\alpine-1.xml
if exist .\src\drivers\berniw\2\a-speedway.xml copy .\src\drivers\berniw\2\a-speedway.xml .\%RUNTIMEDIR%\drivers\berniw\2\a-speedway.xml
if exist .\src\drivers\berniw\2\default.xml copy .\src\drivers\berniw\2\default.xml .\%RUNTIMEDIR%\drivers\berniw\2\default.xml
if exist .\src\drivers\berniw\2\dirt-1.xml copy .\src\drivers\berniw\2\dirt-1.xml .\%RUNTIMEDIR%\drivers\berniw\2\dirt-1.xml
if exist .\src\drivers\berniw\2\dirt-2.xml copy .\src\drivers\berniw\2\dirt-2.xml .\%RUNTIMEDIR%\drivers\berniw\2\dirt-2.xml
if exist .\src\drivers\berniw\2\dirt-3.xml copy .\src\drivers\berniw\2\dirt-3.xml .\%RUNTIMEDIR%\drivers\berniw\2\dirt-3.xml
if exist .\src\drivers\berniw\2\dirt-4.xml copy .\src\drivers\berniw\2\dirt-4.xml .\%RUNTIMEDIR%\drivers\berniw\2\dirt-4.xml
if exist .\src\drivers\berniw\2\dirt-5.xml copy .\src\drivers\berniw\2\dirt-5.xml .\%RUNTIMEDIR%\drivers\berniw\2\dirt-5.xml
if exist .\src\drivers\berniw\2\dirt-6.xml copy .\src\drivers\berniw\2\dirt-6.xml .\%RUNTIMEDIR%\drivers\berniw\2\dirt-6.xml
if exist .\src\drivers\berniw\2\eroad.xml copy .\src\drivers\berniw\2\eroad.xml .\%RUNTIMEDIR%\drivers\berniw\2\eroad.xml
if exist .\src\drivers\berniw\2\e-track-1.xml copy .\src\drivers\berniw\2\e-track-1.xml .\%RUNTIMEDIR%\drivers\berniw\2\e-track-1.xml
if exist .\src\drivers\berniw\2\e-track-2.xml copy .\src\drivers\berniw\2\e-track-2.xml .\%RUNTIMEDIR%\drivers\berniw\2\e-track-2.xml
if exist .\src\drivers\berniw\2\e-track-3.xml copy .\src\drivers\berniw\2\e-track-3.xml .\%RUNTIMEDIR%\drivers\berniw\2\e-track-3.xml
if exist .\src\drivers\berniw\2\e-track-4.xml copy .\src\drivers\berniw\2\e-track-4.xml .\%RUNTIMEDIR%\drivers\berniw\2\e-track-4.xml
if exist .\src\drivers\berniw\2\e-track-5.xml copy .\src\drivers\berniw\2\e-track-5.xml .\%RUNTIMEDIR%\drivers\berniw\2\e-track-5.xml
if exist .\src\drivers\berniw\2\e-track-6.xml copy .\src\drivers\berniw\2\e-track-6.xml .\%RUNTIMEDIR%\drivers\berniw\2\e-track-6.xml
if exist .\src\drivers\berniw\2\g-track-1.xml copy .\src\drivers\berniw\2\g-track-1.xml .\%RUNTIMEDIR%\drivers\berniw\2\g-track-1.xml
if exist .\src\drivers\berniw\2\g-track-2.xml copy .\src\drivers\berniw\2\g-track-2.xml .\%RUNTIMEDIR%\drivers\berniw\2\g-track-2.xml
if exist .\src\drivers\berniw\2\g-track-3.xml copy .\src\drivers\berniw\2\g-track-3.xml .\%RUNTIMEDIR%\drivers\berniw\2\g-track-3.xml
if exist .\src\drivers\berniw\2\michigan.xml copy .\src\drivers\berniw\2\michigan.xml .\%RUNTIMEDIR%\drivers\berniw\2\michigan.xml
if exist .\src\drivers\berniw\2\mixed-1.xml copy .\src\drivers\berniw\2\mixed-1.xml .\%RUNTIMEDIR%\drivers\berniw\2\mixed-1.xml
if exist .\src\drivers\berniw\2\mixed-2.xml copy .\src\drivers\berniw\2\mixed-2.xml .\%RUNTIMEDIR%\drivers\berniw\2\mixed-2.xml
if exist .\src\drivers\berniw\2\spring.xml copy .\src\drivers\berniw\2\spring.xml .\%RUNTIMEDIR%\drivers\berniw\2\spring.xml
if exist .\src\drivers\berniw\2\wheel-1.xml copy .\src\drivers\berniw\2\wheel-1.xml .\%RUNTIMEDIR%\drivers\berniw\2\wheel-1.xml

if exist .\src\drivers\berniw\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\3
if exist .\src\drivers\berniw\3\default.xml copy .\src\drivers\berniw\3\default.xml .\%RUNTIMEDIR%\drivers\berniw\3\default.xml
if exist .\src\drivers\berniw\3\car1-trb1.rgb copy .\src\drivers\berniw\3\car1-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw\3\car1-trb1.rgb

if exist .\src\drivers\berniw\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\4
if exist .\src\drivers\berniw\4\default.xml copy .\src\drivers\berniw\4\default.xml .\%RUNTIMEDIR%\drivers\berniw\4\default.xml
if exist .\src\drivers\berniw\4\car2-trb1.rgb copy .\src\drivers\berniw\4\car2-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw\4\car2-trb1.rgb

if exist .\src\drivers\berniw\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\5
if exist .\src\drivers\berniw\5\car3-trb1.rgb copy .\src\drivers\berniw\5\car3-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw\5\car3-trb1.rgb
if exist .\src\drivers\berniw\5\default.xml copy .\src\drivers\berniw\5\default.xml .\%RUNTIMEDIR%\drivers\berniw\5\default.xml
if exist .\src\drivers\berniw\5\car3-trb1.rgb copy .\src\drivers\berniw\5\car3-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw\5\car3-trb1.rgb

if exist .\src\drivers\berniw\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\6
if exist .\src\drivers\berniw\6\car4-trb1.rgb copy .\src\drivers\berniw\6\car4-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw\6\car4-trb1.rgb
if exist .\src\drivers\berniw\6\default.xml copy .\src\drivers\berniw\6\default.xml .\%RUNTIMEDIR%\drivers\berniw\6\default.xml
if exist .\src\drivers\berniw\6\car4-trb1.rgb copy .\src\drivers\berniw\6\car4-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw\6\car4-trb1.rgb

if exist .\src\drivers\berniw\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\7
if exist .\src\drivers\berniw\7\default.xml copy .\src\drivers\berniw\7\default.xml .\%RUNTIMEDIR%\drivers\berniw\7\default.xml
if exist .\src\drivers\berniw\7\car5-trb1.rgb copy .\src\drivers\berniw\7\car5-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw\7\car5-trb1.rgb

if exist .\src\drivers\berniw\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\8
if exist .\src\drivers\berniw\8\default.xml copy .\src\drivers\berniw\8\default.xml .\%RUNTIMEDIR%\drivers\berniw\8\default.xml
if exist .\src\drivers\berniw\8\car6-trb1.rgb copy .\src\drivers\berniw\8\car6-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw\8\car6-trb1.rgb

if exist .\src\drivers\berniw\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw\9
if exist .\src\drivers\berniw\9\default.xml copy .\src\drivers\berniw\9\default.xml .\%RUNTIMEDIR%\drivers\berniw\9\default.xml
if exist .\src\drivers\berniw\9\car7-trb1.rgb copy .\src\drivers\berniw\9\car7-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw\9\car7-trb1.rgb

if exist .\src\drivers\berniw\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw
if exist .\src\drivers\berniw\berniw.xml copy .\src\drivers\berniw\berniw.xml .\%RUNTIMEDIR%\drivers\berniw\berniw.xml
if exist .\src\drivers\berniw\logo.rgb copy .\src\drivers\berniw\logo.rgb .\%RUNTIMEDIR%\drivers\berniw\logo.rgb

if exist .\src\drivers\berniw2\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\1
if exist .\src\drivers\berniw2\1\default.xml copy .\src\drivers\berniw2\1\default.xml .\%RUNTIMEDIR%\drivers\berniw2\1\default.xml

if exist .\src\drivers\berniw2\10\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\10
if exist .\src\drivers\berniw2\10\car1-trb3.rgb copy .\src\drivers\berniw2\10\car1-trb3.rgb .\%RUNTIMEDIR%\drivers\berniw2\10\car1-trb3.rgb
if exist .\src\drivers\berniw2\10\default.xml copy .\src\drivers\berniw2\10\default.xml .\%RUNTIMEDIR%\drivers\berniw2\10\default.xml

if exist .\src\drivers\berniw2\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\2
if exist .\src\drivers\berniw2\2\default.xml copy .\src\drivers\berniw2\2\default.xml .\%RUNTIMEDIR%\drivers\berniw2\2\default.xml

if exist .\src\drivers\berniw2\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\3
if exist .\src\drivers\berniw2\3\default.xml copy .\src\drivers\berniw2\3\default.xml .\%RUNTIMEDIR%\drivers\berniw2\3\default.xml

if exist .\src\drivers\berniw2\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\4
if exist .\src\drivers\berniw2\4\default.xml copy .\src\drivers\berniw2\4\default.xml .\%RUNTIMEDIR%\drivers\berniw2\4\default.xml

if exist .\src\drivers\berniw2\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\5
if exist .\src\drivers\berniw2\5\default.xml copy .\src\drivers\berniw2\5\default.xml .\%RUNTIMEDIR%\drivers\berniw2\5\default.xml

if exist .\src\drivers\berniw2\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\6
if exist .\src\drivers\berniw2\6\default.xml copy .\src\drivers\berniw2\6\default.xml .\%RUNTIMEDIR%\drivers\berniw2\6\default.xml

if exist .\src\drivers\berniw2\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\7
if exist .\src\drivers\berniw2\7\car5-trb1.rgb copy .\src\drivers\berniw2\7\car5-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw2\7\car5-trb1.rgb
if exist .\src\drivers\berniw2\7\default.xml copy .\src\drivers\berniw2\7\default.xml .\%RUNTIMEDIR%\drivers\berniw2\7\default.xml

if exist .\src\drivers\berniw2\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\8
if exist .\src\drivers\berniw2\8\car4-trb1.rgb copy .\src\drivers\berniw2\8\car4-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw2\8\car4-trb1.rgb
if exist .\src\drivers\berniw2\8\default.xml copy .\src\drivers\berniw2\8\default.xml .\%RUNTIMEDIR%\drivers\berniw2\8\default.xml

if exist .\src\drivers\berniw2\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2\9
if exist .\src\drivers\berniw2\9\default.xml copy .\src\drivers\berniw2\9\default.xml .\%RUNTIMEDIR%\drivers\berniw2\9\default.xml
if exist .\src\drivers\berniw2\9\car7-trb1.rgb copy .\src\drivers\berniw2\9\car7-trb1.rgb .\%RUNTIMEDIR%\drivers\berniw2\9\car7-trb1.rgb

if exist .\src\drivers\berniw2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2
if exist .\src\drivers\berniw2\berniw2.xml copy .\src\drivers\berniw2\berniw2.xml .\%RUNTIMEDIR%\drivers\berniw2\berniw2.xml
if exist .\src\drivers\berniw2\logo.rgb copy .\src\drivers\berniw2\logo.rgb .\%RUNTIMEDIR%\drivers\berniw2\logo.rgb

if exist .\src\drivers\berniw3\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\1
if exist .\src\drivers\berniw3\1\default.xml copy .\src\drivers\berniw3\1\default.xml .\%RUNTIMEDIR%\drivers\berniw3\1\default.xml

if exist .\src\drivers\berniw3\10\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\10
if exist .\src\drivers\berniw3\10\default.xml copy .\src\drivers\berniw3\10\default.xml .\%RUNTIMEDIR%\drivers\berniw3\10\default.xml

if exist .\src\drivers\berniw3\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\2
if exist .\src\drivers\berniw3\2\default.xml copy .\src\drivers\berniw3\2\default.xml .\%RUNTIMEDIR%\drivers\berniw3\2\default.xml

if exist .\src\drivers\berniw3\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\3
if exist .\src\drivers\berniw3\3\default.xml copy .\src\drivers\berniw3\3\default.xml .\%RUNTIMEDIR%\drivers\berniw3\3\default.xml

if exist .\src\drivers\berniw3\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\4
if exist .\src\drivers\berniw3\4\default.xml copy .\src\drivers\berniw3\4\default.xml .\%RUNTIMEDIR%\drivers\berniw3\4\default.xml

if exist .\src\drivers\berniw3\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\5
if exist .\src\drivers\berniw3\5\default.xml copy .\src\drivers\berniw3\5\default.xml .\%RUNTIMEDIR%\drivers\berniw3\5\default.xml

if exist .\src\drivers\berniw3\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\6
if exist .\src\drivers\berniw3\6\default.xml copy .\src\drivers\berniw3\6\default.xml .\%RUNTIMEDIR%\drivers\berniw3\6\default.xml

if exist .\src\drivers\berniw3\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\7
if exist .\src\drivers\berniw3\7\default.xml copy .\src\drivers\berniw3\7\default.xml .\%RUNTIMEDIR%\drivers\berniw3\7\default.xml

if exist .\src\drivers\berniw3\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\8
if exist .\src\drivers\berniw3\8\default.xml copy .\src\drivers\berniw3\8\default.xml .\%RUNTIMEDIR%\drivers\berniw3\8\default.xml

if exist .\src\drivers\berniw3\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3\9
if exist .\src\drivers\berniw3\9\default.xml copy .\src\drivers\berniw3\9\default.xml .\%RUNTIMEDIR%\drivers\berniw3\9\default.xml

if exist .\src\drivers\berniw3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\berniw3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\berniw3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3
if exist .\src\drivers\berniw3\berniw3.xml copy .\src\drivers\berniw3\berniw3.xml .\%RUNTIMEDIR%\drivers\berniw3\berniw3.xml
if exist .\src\drivers\berniw3\logo.rgb copy .\src\drivers\berniw3\logo.rgb .\%RUNTIMEDIR%\drivers\berniw3\logo.rgb


if exist .\src\drivers\bt\0\practice\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\0\practice\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\0\practice\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\0\practice\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\0
if exist .\src\drivers\bt\0\practice\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\0\practice
if exist .\src\drivers\bt\0\practice\g-track-3.xml copy .\src\drivers\bt\0\practice\g-track-3.xml .\%RUNTIMEDIR%\drivers\bt\0\practice\g-track-3.xml
if exist .\src\drivers\bt\0\practice\michigan.xml copy .\src\drivers\bt\0\practice\michigan.xml .\%RUNTIMEDIR%\drivers\bt\0\practice\michigan.xml
if exist .\src\drivers\bt\0\practice\wheel-1.xml copy .\src\drivers\bt\0\practice\wheel-1.xml .\%RUNTIMEDIR%\drivers\bt\0\practice\wheel-1.xml

if exist .\src\drivers\bt\0\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\0\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\0\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\0\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\0
if exist .\src\drivers\bt\0\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\0\qualifying
if exist .\src\drivers\bt\0\qualifying\g-track-3.xml copy .\src\drivers\bt\0\qualifying\g-track-3.xml .\%RUNTIMEDIR%\drivers\bt\0\qualifying\g-track-3.xml

if exist .\src\drivers\bt\0\race\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\0\race\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\0\race\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\0\race\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\0
if exist .\src\drivers\bt\0\race\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\0\race
if exist .\src\drivers\bt\0\race\g-track-3.xml copy .\src\drivers\bt\0\race\g-track-3.xml .\%RUNTIMEDIR%\drivers\bt\0\race\g-track-3.xml
if exist .\src\drivers\bt\0\race\wheel-1.xml copy .\src\drivers\bt\0\race\wheel-1.xml .\%RUNTIMEDIR%\drivers\bt\0\race\wheel-1.xml

if exist .\src\drivers\bt\0\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\0
if exist .\src\drivers\bt\0\default.xml copy .\src\drivers\bt\0\default.xml .\%RUNTIMEDIR%\drivers\bt\0\default.xml
if exist .\src\drivers\bt\0\cg-nascar-rwd.rgb copy .\src\drivers\bt\0\cg-nascar-rwd.rgb .\%RUNTIMEDIR%\drivers\bt\0\cg-nascar-rwd.rgb

if exist .\src\drivers\bt\1\practice\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\1\practice\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\1\practice\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\1\practice\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\1
if exist .\src\drivers\bt\1\practice\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\1\practice
if exist .\src\drivers\bt\1\practice\g-track-3.xml copy .\src\drivers\bt\1\practice\g-track-3.xml .\%RUNTIMEDIR%\drivers\bt\1\practice\g-track-3.xml
if exist .\src\drivers\bt\1\practice\michigan.xml copy .\src\drivers\bt\1\practice\michigan.xml .\%RUNTIMEDIR%\drivers\bt\1\practice\michigan.xml
if exist .\src\drivers\bt\1\practice\wheel-1.xml copy .\src\drivers\bt\1\practice\wheel-1.xml .\%RUNTIMEDIR%\drivers\bt\1\practice\wheel-1.xml

if exist .\src\drivers\bt\1\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\1\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\1\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\1\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\1
if exist .\src\drivers\bt\1\qualifying\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\1\qualifying
if exist .\src\drivers\bt\1\qualifying\g-track-3.xml copy .\src\drivers\bt\1\qualifying\g-track-3.xml .\%RUNTIMEDIR%\drivers\bt\1\qualifying\g-track-3.xml

if exist .\src\drivers\bt\1\race\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\1\race\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\1\race\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\1\race\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\1
if exist .\src\drivers\bt\1\race\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\1\race
if exist .\src\drivers\bt\1\race\g-track-3.xml copy .\src\drivers\bt\1\race\g-track-3.xml .\%RUNTIMEDIR%\drivers\bt\1\race\g-track-3.xml
if exist .\src\drivers\bt\1\race\wheel-1.xml copy .\src\drivers\bt\1\race\wheel-1.xml .\%RUNTIMEDIR%\drivers\bt\1\race\wheel-1.xml

if exist .\src\drivers\bt\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\1
if exist .\src\drivers\bt\1\default.xml copy .\src\drivers\bt\1\default.xml .\%RUNTIMEDIR%\drivers\bt\1\default.xml
if exist .\src\drivers\bt\1\cg-nascar-rwd.rgb copy .\src\drivers\bt\1\cg-nascar-rwd.rgb .\%RUNTIMEDIR%\drivers\bt\1\cg-nascar-rwd.rgb

if exist .\src\drivers\bt\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\2
if exist .\src\drivers\bt\2\default.xml copy .\src\drivers\bt\2\default.xml .\%RUNTIMEDIR%\drivers\bt\2\default.xml
if exist .\src\drivers\bt\2\car1-trb1.rgb copy .\src\drivers\bt\2\car1-trb1.rgb .\%RUNTIMEDIR%\drivers\bt\2\car1-trb1.rgb

if exist .\src\drivers\bt\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\3
if exist .\src\drivers\bt\3\default.xml copy .\src\drivers\bt\3\default.xml .\%RUNTIMEDIR%\drivers\bt\3\default.xml
if exist .\src\drivers\bt\3\car2-trb1.rgb copy .\src\drivers\bt\3\car2-trb1.rgb .\%RUNTIMEDIR%\drivers\bt\3\car2-trb1.rgb

if exist .\src\drivers\bt\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\4
if exist .\src\drivers\bt\4\default.xml copy .\src\drivers\bt\4\default.xml .\%RUNTIMEDIR%\drivers\bt\4\default.xml
if exist .\src\drivers\bt\4\car3-trb1.rgb copy .\src\drivers\bt\4\car3-trb1.rgb .\%RUNTIMEDIR%\drivers\bt\4\car3-trb1.rgb

if exist .\src\drivers\bt\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\5
if exist .\src\drivers\bt\5\default.xml copy .\src\drivers\bt\5\default.xml .\%RUNTIMEDIR%\drivers\bt\5\default.xml
if exist .\src\drivers\bt\5\car4-trb1.rgb copy .\src\drivers\bt\5\car4-trb1.rgb .\%RUNTIMEDIR%\drivers\bt\5\car4-trb1.rgb

if exist .\src\drivers\bt\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\6
if exist .\src\drivers\bt\6\default.xml copy .\src\drivers\bt\6\default.xml .\%RUNTIMEDIR%\drivers\bt\6\default.xml
if exist .\src\drivers\bt\6\car5-trb1.rgb copy .\src\drivers\bt\6\car5-trb1.rgb .\%RUNTIMEDIR%\drivers\bt\6\car5-trb1.rgb

if exist .\src\drivers\bt\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\7
if exist .\src\drivers\bt\7\default.xml copy .\src\drivers\bt\7\default.xml .\%RUNTIMEDIR%\drivers\bt\7\default.xml
if exist .\src\drivers\bt\7\car6-trb1.rgb copy .\src\drivers\bt\7\car6-trb1.rgb .\%RUNTIMEDIR%\drivers\bt\7\car6-trb1.rgb

if exist .\src\drivers\bt\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\8
if exist .\src\drivers\bt\8\default.xml copy .\src\drivers\bt\8\default.xml .\%RUNTIMEDIR%\drivers\bt\8\default.xml
if exist .\src\drivers\bt\8\car7-trb1.rgb copy .\src\drivers\bt\8\car7-trb1.rgb .\%RUNTIMEDIR%\drivers\bt\8\car7-trb1.rgb

if exist .\src\drivers\bt\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt\9
if exist .\src\drivers\bt\9\default.xml copy .\src\drivers\bt\9\default.xml .\%RUNTIMEDIR%\drivers\bt\9\default.xml

if exist .\src\drivers\bt\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\bt\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\bt\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\bt
if exist .\src\drivers\bt\bt.xml copy .\src\drivers\bt\bt.xml .\%RUNTIMEDIR%\drivers\bt\bt.xml
if exist .\src\drivers\bt\logo.rgb copy .\src\drivers\bt\logo.rgb .\%RUNTIMEDIR%\drivers\bt\logo.rgb

if exist .\src\drivers\damned\0\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\0
if exist .\src\drivers\damned\0\default.xml copy .\src\drivers\damned\0\default.xml .\%RUNTIMEDIR%\drivers\damned\0\default.xml

if exist .\src\drivers\damned\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\1
if exist .\src\drivers\damned\1\default.xml copy .\src\drivers\damned\1\default.xml .\%RUNTIMEDIR%\drivers\damned\1\default.xml

if exist .\src\drivers\damned\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\2
if exist .\src\drivers\damned\2\default.xml copy .\src\drivers\damned\2\default.xml .\%RUNTIMEDIR%\drivers\damned\2\default.xml

if exist .\src\drivers\damned\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\3
if exist .\src\drivers\damned\3\default.xml copy .\src\drivers\damned\3\default.xml .\%RUNTIMEDIR%\drivers\damned\3\default.xml

if exist .\src\drivers\damned\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\4
if exist .\src\drivers\damned\4\default.xml copy .\src\drivers\damned\4\default.xml .\%RUNTIMEDIR%\drivers\damned\4\default.xml

if exist .\src\drivers\damned\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\5
if exist .\src\drivers\damned\5\default.xml copy .\src\drivers\damned\5\default.xml .\%RUNTIMEDIR%\drivers\damned\5\default.xml

if exist .\src\drivers\damned\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\6
if exist .\src\drivers\damned\6\default.xml copy .\src\drivers\damned\6\default.xml .\%RUNTIMEDIR%\drivers\damned\6\default.xml
if exist .\src\drivers\damned\6\car5-trb1.rgb copy .\src\drivers\damned\6\car5-trb1.rgb .\%RUNTIMEDIR%\drivers\damned\6\car5-trb1.rgb

if exist .\src\drivers\damned\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\7
if exist .\src\drivers\damned\7\default.xml copy .\src\drivers\damned\7\default.xml .\%RUNTIMEDIR%\drivers\damned\7\default.xml
if exist .\src\drivers\damned\7\car6-trb1.rgb copy .\src\drivers\damned\7\car6-trb1.rgb .\%RUNTIMEDIR%\drivers\damned\7\car6-trb1.rgb

if exist .\src\drivers\damned\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\8
if exist .\src\drivers\damned\8\default.xml copy .\src\drivers\damned\8\default.xml .\%RUNTIMEDIR%\drivers\damned\8\default.xml
if exist .\src\drivers\damned\8\car7-trb1.rgb copy .\src\drivers\damned\8\car7-trb1.rgb .\%RUNTIMEDIR%\drivers\damned\8\car7-trb1.rgb

if exist .\src\drivers\damned\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned\9
if exist .\src\drivers\damned\9\default.xml copy .\src\drivers\damned\9\default.xml .\%RUNTIMEDIR%\drivers\damned\9\default.xml
if exist .\src\drivers\damned\9\car1-trb3.rgb copy .\src\drivers\damned\9\car1-trb3.rgb .\%RUNTIMEDIR%\drivers\damned\9\car1-trb3.rgb

if exist .\src\drivers\damned\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\damned\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\damned\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\damned
if exist .\src\drivers\damned\damned.xml copy .\src\drivers\damned\damned.xml .\%RUNTIMEDIR%\drivers\damned\damned.xml
if exist .\src\drivers\damned\logo.rgb copy .\src\drivers\damned\logo.rgb .\%RUNTIMEDIR%\drivers\damned\logo.rgb

if exist .\src\drivers\human\tracks\b-speedway\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\human\tracks\b-speedway\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\human\tracks\b-speedway\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\human
if exist .\src\drivers\human\tracks\b-speedway\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\human\tracks
if exist .\src\drivers\human\tracks\b-speedway\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\human\tracks\b-speedway
if exist .\src\drivers\human\tracks\b-speedway\car-porsche-gt1.xml copy .\src\drivers\human\tracks\b-speedway\car-porsche-gt1.xml .\%RUNTIMEDIR%\drivers\human\tracks\b-speedway\car-porsche-gt1.xml

if exist .\src\drivers\human\tracks\dirt-1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\human\tracks\dirt-1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\human\tracks\dirt-1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\human
if exist .\src\drivers\human\tracks\dirt-1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\human\tracks
if exist .\src\drivers\human\tracks\dirt-1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\human\tracks\dirt-1

if exist .\src\drivers\human\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\human\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\human\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\human
if exist .\src\drivers\human\car.xml copy .\src\drivers\human\car.xml .\%RUNTIMEDIR%\drivers\human\car.xml
if exist .\src\drivers\human\human.xml copy .\src\drivers\human\human.xml .\%RUNTIMEDIR%\drivers\human\human.xml
if exist .\src\drivers\human\preferences.xml copy .\src\drivers\human\preferences.xml .\%RUNTIMEDIR%\drivers\human\preferences.xml
if exist .\src\drivers\human\logo.rgb copy .\src\drivers\human\logo.rgb .\%RUNTIMEDIR%\drivers\human\logo.rgb

if exist .\src\drivers\inferno\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\1
if exist .\src\drivers\inferno\1\default.xml copy .\src\drivers\inferno\1\default.xml .\%RUNTIMEDIR%\drivers\inferno\1\default.xml

if exist .\src\drivers\inferno\10\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\10
if exist .\src\drivers\inferno\10\default.xml copy .\src\drivers\inferno\10\default.xml .\%RUNTIMEDIR%\drivers\inferno\10\default.xml
if exist .\src\drivers\inferno\10\car1-trb3.rgb copy .\src\drivers\inferno\10\car1-trb3.rgb .\%RUNTIMEDIR%\drivers\inferno\10\car1-trb3.rgb

if exist .\src\drivers\inferno\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\2
if exist .\src\drivers\inferno\2\default.xml copy .\src\drivers\inferno\2\default.xml .\%RUNTIMEDIR%\drivers\inferno\2\default.xml
if exist .\src\drivers\inferno\2\p406.rgb copy .\src\drivers\inferno\2\p406.rgb .\%RUNTIMEDIR%\drivers\inferno\2\p406.rgb

if exist .\src\drivers\inferno\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\3
if exist .\src\drivers\inferno\3\default.xml copy .\src\drivers\inferno\3\default.xml .\%RUNTIMEDIR%\drivers\inferno\3\default.xml
if exist .\src\drivers\inferno\3\car1-trb1.rgb copy .\src\drivers\inferno\3\car1-trb1.rgb .\%RUNTIMEDIR%\drivers\inferno\3\car1-trb1.rgb

if exist .\src\drivers\inferno\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\4
if exist .\src\drivers\inferno\4\default.xml copy .\src\drivers\inferno\4\default.xml .\%RUNTIMEDIR%\drivers\inferno\4\default.xml
if exist .\src\drivers\inferno\4\car2-trb1.rgb copy .\src\drivers\inferno\4\car2-trb1.rgb .\%RUNTIMEDIR%\drivers\inferno\4\car2-trb1.rgb

if exist .\src\drivers\inferno\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\5
if exist .\src\drivers\inferno\5\default.xml copy .\src\drivers\inferno\5\default.xml .\%RUNTIMEDIR%\drivers\inferno\5\default.xml
if exist .\src\drivers\inferno\5\car3-trb1.rgb copy .\src\drivers\inferno\5\car3-trb1.rgb .\%RUNTIMEDIR%\drivers\inferno\5\car3-trb1.rgb

if exist .\src\drivers\inferno\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\6
if exist .\src\drivers\inferno\6\default.xml copy .\src\drivers\inferno\6\default.xml .\%RUNTIMEDIR%\drivers\inferno\6\default.xml
if exist .\src\drivers\inferno\6\car4-trb1.rgb copy .\src\drivers\inferno\6\car4-trb1.rgb .\%RUNTIMEDIR%\drivers\inferno\6\car4-trb1.rgb

if exist .\src\drivers\inferno\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\7
if exist .\src\drivers\inferno\7\default.xml copy .\src\drivers\inferno\7\default.xml .\%RUNTIMEDIR%\drivers\inferno\7\default.xml
if exist .\src\drivers\inferno\7\car5-trb1.rgb copy .\src\drivers\inferno\7\car5-trb1.rgb .\%RUNTIMEDIR%\drivers\inferno\7\car5-trb1.rgb

if exist .\src\drivers\inferno\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\8
if exist .\src\drivers\inferno\8\default.xml copy .\src\drivers\inferno\8\default.xml .\%RUNTIMEDIR%\drivers\inferno\8\default.xml
if exist .\src\drivers\inferno\8\car6-trb1.rgb copy .\src\drivers\inferno\8\car6-trb1.rgb .\%RUNTIMEDIR%\drivers\inferno\8\car6-trb1.rgb

if exist .\src\drivers\inferno\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno\9
if exist .\src\drivers\inferno\9\default.xml copy .\src\drivers\inferno\9\default.xml .\%RUNTIMEDIR%\drivers\inferno\9\default.xml
if exist .\src\drivers\inferno\9\car7-trb1.rgb copy .\src\drivers\inferno\9\car7-trb1.rgb .\%RUNTIMEDIR%\drivers\inferno\9\car7-trb1.rgb

if exist .\src\drivers\inferno\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno
if exist .\src\drivers\inferno\inferno.xml copy .\src\drivers\inferno\inferno.xml .\%RUNTIMEDIR%\drivers\inferno\inferno.xml
if exist .\src\drivers\inferno\logo.rgb copy .\src\drivers\inferno\logo.rgb .\%RUNTIMEDIR%\drivers\inferno\logo.rgb

if exist .\src\drivers\inferno2\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\1
if exist .\src\drivers\inferno2\1\defaultcar.xml copy .\src\drivers\inferno2\1\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\1\defaultcar.xml
if exist .\src\drivers\inferno2\1\default.xml copy .\src\drivers\inferno2\1\default.xml .\%RUNTIMEDIR%\drivers\inferno2\1\default.xml

if exist .\src\drivers\inferno2\10\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\10
if exist .\src\drivers\inferno2\10\defaultcar.xml copy .\src\drivers\inferno2\10\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\10\defaultcar.xml
if exist .\src\drivers\inferno2\10\default.xml copy .\src\drivers\inferno2\10\default.xml .\%RUNTIMEDIR%\drivers\inferno2\10\default.xml

if exist .\src\drivers\inferno2\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\2
if exist .\src\drivers\inferno2\2\defaultcar.xml copy .\src\drivers\inferno2\2\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\2\defaultcar.xml
if exist .\src\drivers\inferno2\2\default.xml copy .\src\drivers\inferno2\2\default.xml .\%RUNTIMEDIR%\drivers\inferno2\2\default.xml

if exist .\src\drivers\inferno2\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\3
if exist .\src\drivers\inferno2\3\defaultcar.xml copy .\src\drivers\inferno2\3\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\3\defaultcar.xml
if exist .\src\drivers\inferno2\3\default.xml copy .\src\drivers\inferno2\3\default.xml .\%RUNTIMEDIR%\drivers\inferno2\3\default.xml

if exist .\src\drivers\inferno2\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\4
if exist .\src\drivers\inferno2\4\defaultcar.xml copy .\src\drivers\inferno2\4\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\4\defaultcar.xml
if exist .\src\drivers\inferno2\4\default.xml copy .\src\drivers\inferno2\4\default.xml .\%RUNTIMEDIR%\drivers\inferno2\4\default.xml

if exist .\src\drivers\inferno2\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\5
if exist .\src\drivers\inferno2\5\defaultcar.xml copy .\src\drivers\inferno2\5\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\5\defaultcar.xml
if exist .\src\drivers\inferno2\5\default.xml copy .\src\drivers\inferno2\5\default.xml .\%RUNTIMEDIR%\drivers\inferno2\5\default.xml

if exist .\src\drivers\inferno2\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\6
if exist .\src\drivers\inferno2\6\defaultcar.xml copy .\src\drivers\inferno2\6\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\6\defaultcar.xml
if exist .\src\drivers\inferno2\6\default.xml copy .\src\drivers\inferno2\6\default.xml .\%RUNTIMEDIR%\drivers\inferno2\6\default.xml

if exist .\src\drivers\inferno2\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\7
if exist .\src\drivers\inferno2\7\defaultcar.xml copy .\src\drivers\inferno2\7\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\7\defaultcar.xml
if exist .\src\drivers\inferno2\7\default.xml copy .\src\drivers\inferno2\7\default.xml .\%RUNTIMEDIR%\drivers\inferno2\7\default.xml

if exist .\src\drivers\inferno2\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\8
if exist .\src\drivers\inferno2\8\defaultcar.xml copy .\src\drivers\inferno2\8\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\8\defaultcar.xml
if exist .\src\drivers\inferno2\8\default.xml copy .\src\drivers\inferno2\8\default.xml .\%RUNTIMEDIR%\drivers\inferno2\8\default.xml

if exist .\src\drivers\inferno2\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2\9
if exist .\src\drivers\inferno2\9\defaultcar.xml copy .\src\drivers\inferno2\9\defaultcar.xml .\%RUNTIMEDIR%\drivers\inferno2\9\defaultcar.xml
if exist .\src\drivers\inferno2\9\default.xml copy .\src\drivers\inferno2\9\default.xml .\%RUNTIMEDIR%\drivers\inferno2\9\default.xml

if exist .\src\drivers\inferno2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\inferno2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\inferno2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2
if exist .\src\drivers\inferno2\inferno2.xml copy .\src\drivers\inferno2\inferno2.xml .\%RUNTIMEDIR%\drivers\inferno2\inferno2.xml
if exist .\src\drivers\inferno2\logo.rgb copy .\src\drivers\inferno2\logo.rgb .\%RUNTIMEDIR%\drivers\inferno2\logo.rgb

if exist .\src\drivers\sparkle\0\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\sparkle\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\sparkle\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\sparkle
if exist .\src\drivers\sparkle\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\sparkle\0
if exist .\src\drivers\sparkle\0\baja-bug.rgb copy .\src\drivers\sparkle\0\baja-bug.rgb .\%RUNTIMEDIR%\drivers\sparkle\0\baja-bug.rgb
if exist .\src\drivers\sparkle\0\default.xml copy .\src\drivers\sparkle\0\default.xml .\%RUNTIMEDIR%\drivers\sparkle\0\default.xml

if exist .\src\drivers\sparkle\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\sparkle\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\sparkle\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\sparkle
if exist .\src\drivers\sparkle\sparkle.xml copy .\src\drivers\sparkle\sparkle.xml .\%RUNTIMEDIR%\drivers\sparkle\sparkle.xml
if exist .\src\drivers\sparkle\logo.rgb copy .\src\drivers\sparkle\logo.rgb .\%RUNTIMEDIR%\drivers\sparkle\logo.rgb

if exist .\src\drivers\tita\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\1
if exist .\src\drivers\tita\1\default.xml copy .\src\drivers\tita\1\default.xml .\%RUNTIMEDIR%\drivers\tita\1\default.xml

if exist .\src\drivers\tita\10\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\10
if exist .\src\drivers\tita\10\default.xml copy .\src\drivers\tita\10\default.xml .\%RUNTIMEDIR%\drivers\tita\10\default.xml

if exist .\src\drivers\tita\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\2
if exist .\src\drivers\tita\2\default.xml copy .\src\drivers\tita\2\default.xml .\%RUNTIMEDIR%\drivers\tita\2\default.xml

if exist .\src\drivers\tita\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\3
if exist .\src\drivers\tita\3\default.xml copy .\src\drivers\tita\3\default.xml .\%RUNTIMEDIR%\drivers\tita\3\default.xml
if exist .\src\drivers\tita\3\car1-trb1.rgb copy .\src\drivers\tita\3\car1-trb1.rgb .\%RUNTIMEDIR%\drivers\tita\3\car1-trb1.rgb

if exist .\src\drivers\tita\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\4
if exist .\src\drivers\tita\4\default.xml copy .\src\drivers\tita\4\default.xml .\%RUNTIMEDIR%\drivers\tita\4\default.xml
if exist .\src\drivers\tita\4\car2-trb1.rgb copy .\src\drivers\tita\4\car2-trb1.rgb .\%RUNTIMEDIR%\drivers\tita\4\car2-trb1.rgb

if exist .\src\drivers\tita\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\5
if exist .\src\drivers\tita\5\default.xml copy .\src\drivers\tita\5\default.xml .\%RUNTIMEDIR%\drivers\tita\5\default.xml
if exist .\src\drivers\tita\5\car3-trb1.rgb copy .\src\drivers\tita\5\car3-trb1.rgb .\%RUNTIMEDIR%\drivers\tita\5\car3-trb1.rgb

if exist .\src\drivers\tita\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\6
if exist .\src\drivers\tita\6\default.xml copy .\src\drivers\tita\6\default.xml .\%RUNTIMEDIR%\drivers\tita\6\default.xml
if exist .\src\drivers\tita\6\car4-trb1.rgb copy .\src\drivers\tita\6\car4-trb1.rgb .\%RUNTIMEDIR%\drivers\tita\6\car4-trb1.rgb

if exist .\src\drivers\tita\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\7
if exist .\src\drivers\tita\7\default.xml copy .\src\drivers\tita\7\default.xml .\%RUNTIMEDIR%\drivers\tita\7\default.xml
if exist .\src\drivers\tita\7\car5-trb1.rgb copy .\src\drivers\tita\7\car5-trb1.rgb .\%RUNTIMEDIR%\drivers\tita\7\car5-trb1.rgb

if exist .\src\drivers\tita\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\8
if exist .\src\drivers\tita\8\default.xml copy .\src\drivers\tita\8\default.xml .\%RUNTIMEDIR%\drivers\tita\8\default.xml
if exist .\src\drivers\tita\8\car6-trb1.rgb copy .\src\drivers\tita\8\car6-trb1.rgb .\%RUNTIMEDIR%\drivers\tita\8\car6-trb1.rgb

if exist .\src\drivers\tita\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita\9
if exist .\src\drivers\tita\9\default.xml copy .\src\drivers\tita\9\default.xml .\%RUNTIMEDIR%\drivers\tita\9\default.xml
if exist .\src\drivers\tita\9\car7-trb1.rgb copy .\src\drivers\tita\9\car7-trb1.rgb .\%RUNTIMEDIR%\drivers\tita\9\car7-trb1.rgb

if exist .\src\drivers\tita\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\tita\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\tita\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\tita
if exist .\src\drivers\tita\tita.xml copy .\src\drivers\tita\tita.xml .\%RUNTIMEDIR%\drivers\tita\tita.xml
if exist .\src\drivers\tita\logo.rgb copy .\src\drivers\tita\logo.rgb .\%RUNTIMEDIR%\drivers\tita\logo.rgb

if exist .\src\drivers\lliaw\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\1
if exist .\src\drivers\lliaw\1\default.xml copy .\src\drivers\lliaw\1\default.xml .\%RUNTIMEDIR%\drivers\lliaw\1\default.xml

if exist .\src\drivers\lliaw\10\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\10\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\10
if exist .\src\drivers\lliaw\10\default.xml copy .\src\drivers\lliaw\10\default.xml .\%RUNTIMEDIR%\drivers\lliaw\10\default.xml

if exist .\src\drivers\lliaw\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\2
if exist .\src\drivers\lliaw\2\default.xml copy .\src\drivers\lliaw\2\default.xml .\%RUNTIMEDIR%\drivers\lliaw\2\default.xml

if exist .\src\drivers\lliaw\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\3
if exist .\src\drivers\lliaw\3\default.xml copy .\src\drivers\lliaw\3\default.xml .\%RUNTIMEDIR%\drivers\lliaw\3\default.xml
if exist .\src\drivers\lliaw\3\car1-trb1.rgb copy .\src\drivers\lliaw\3\car1-trb1.rgb .\%RUNTIMEDIR%\drivers\lliaw\3\car1-trb1.rgb

if exist .\src\drivers\lliaw\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\4
if exist .\src\drivers\lliaw\4\default.xml copy .\src\drivers\lliaw\4\default.xml .\%RUNTIMEDIR%\drivers\lliaw\4\default.xml
if exist .\src\drivers\lliaw\4\car2-trb1.rgb copy .\src\drivers\lliaw\4\car2-trb1.rgb .\%RUNTIMEDIR%\drivers\lliaw\4\car2-trb1.rgb

if exist .\src\drivers\lliaw\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\5
if exist .\src\drivers\lliaw\5\default.xml copy .\src\drivers\lliaw\5\default.xml .\%RUNTIMEDIR%\drivers\lliaw\5\default.xml
if exist .\src\drivers\lliaw\5\car3-trb1.rgb copy .\src\drivers\lliaw\5\car3-trb1.rgb .\%RUNTIMEDIR%\drivers\lliaw\5\car3-trb1.rgb

if exist .\src\drivers\lliaw\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\6
if exist .\src\drivers\lliaw\6\default.xml copy .\src\drivers\lliaw\6\default.xml .\%RUNTIMEDIR%\drivers\lliaw\6\default.xml
if exist .\src\drivers\lliaw\6\car4-trb1.rgb copy .\src\drivers\lliaw\6\car4-trb1.rgb .\%RUNTIMEDIR%\drivers\lliaw\6\car4-trb1.rgb

if exist .\src\drivers\lliaw\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\7
if exist .\src\drivers\lliaw\7\default.xml copy .\src\drivers\lliaw\7\default.xml .\%RUNTIMEDIR%\drivers\lliaw\7\default.xml
if exist .\src\drivers\lliaw\7\car5-trb1.rgb copy .\src\drivers\lliaw\7\car5-trb1.rgb .\%RUNTIMEDIR%\drivers\lliaw\7\car5-trb1.rgb

if exist .\src\drivers\lliaw\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\8
if exist .\src\drivers\lliaw\8\default.xml copy .\src\drivers\lliaw\8\default.xml .\%RUNTIMEDIR%\drivers\lliaw\8\default.xml
if exist .\src\drivers\lliaw\8\car6-trb1.rgb copy .\src\drivers\lliaw\8\car6-trb1.rgb .\%RUNTIMEDIR%\drivers\lliaw\8\car6-trb1.rgb

if exist .\src\drivers\lliaw\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw\9
if exist .\src\drivers\lliaw\9\default.xml copy .\src\drivers\lliaw\9\default.xml .\%RUNTIMEDIR%\drivers\lliaw\9\default.xml
if exist .\src\drivers\lliaw\9\car7-trb1.rgb copy .\src\drivers\lliaw\9\car7-trb1.rgb .\%RUNTIMEDIR%\drivers\lliaw\9\car7-trb1.rgb


if exist .\src\drivers\lliaw\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\lliaw\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\lliaw\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw
if exist .\src\drivers\lliaw\lliaw.xml copy .\src\drivers\lliaw\lliaw.xml .\%RUNTIMEDIR%\drivers\lliaw\lliaw.xml
if exist .\src\drivers\lliaw\logo.rgb copy .\src\drivers\lliaw\logo.rgb .\%RUNTIMEDIR%\drivers\lliaw\logo.rgb


if exist .\src\drivers\olethros\0\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\0\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\0
if exist .\src\drivers\olethros\0\default.xml copy .\src\drivers\olethros\0\default.xml .\%RUNTIMEDIR%\drivers\olethros\0\default.xml

if exist .\src\drivers\olethros\1\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\1\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\1
if exist .\src\drivers\olethros\1\default.xml copy .\src\drivers\olethros\1\default.xml .\%RUNTIMEDIR%\drivers\olethros\1\default.xml

if exist .\src\drivers\olethros\2\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\2\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\2
if exist .\src\drivers\olethros\2\default.xml copy .\src\drivers\olethros\2\default.xml .\%RUNTIMEDIR%\drivers\olethros\2\default.xml
if exist .\src\drivers\olethros\2\car1-trb1.rgb copy .\src\drivers\olethros\2\car1-trb1.rgb .\%RUNTIMEDIR%\drivers\olethros\2\car1-trb1.rgb

if exist .\src\drivers\olethros\3\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\3\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\3
if exist .\src\drivers\olethros\3\default.xml copy .\src\drivers\olethros\3\default.xml .\%RUNTIMEDIR%\drivers\olethros\3\default.xml
if exist .\src\drivers\olethros\3\car2-trb1.rgb copy .\src\drivers\olethros\3\car2-trb1.rgb .\%RUNTIMEDIR%\drivers\olethros\3\car2-trb1.rgb

if exist .\src\drivers\olethros\4\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\4\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\4
if exist .\src\drivers\olethros\4\default.xml copy .\src\drivers\olethros\4\default.xml .\%RUNTIMEDIR%\drivers\olethros\4\default.xml
if exist .\src\drivers\olethros\4\car3-trb1.rgb copy .\src\drivers\olethros\4\car3-trb1.rgb .\%RUNTIMEDIR%\drivers\olethros\4\car3-trb1.rgb

if exist .\src\drivers\olethros\5\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\5\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\5
if exist .\src\drivers\olethros\5\default.xml copy .\src\drivers\olethros\5\default.xml .\%RUNTIMEDIR%\drivers\olethros\5\default.xml
if exist .\src\drivers\olethros\5\car4-trb1.rgb copy .\src\drivers\olethros\5\car4-trb1.rgb .\%RUNTIMEDIR%\drivers\olethros\5\car4-trb1.rgb

if exist .\src\drivers\olethros\6\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\6\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\6
if exist .\src\drivers\olethros\6\default.xml copy .\src\drivers\olethros\6\default.xml .\%RUNTIMEDIR%\drivers\olethros\6\default.xml
if exist .\src\drivers\olethros\6\car5-trb1.rgb copy .\src\drivers\olethros\6\car5-trb1.rgb .\%RUNTIMEDIR%\drivers\olethros\6\car5-trb1.rgb

if exist .\src\drivers\olethros\7\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\7\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\7
if exist .\src\drivers\olethros\7\default.xml copy .\src\drivers\olethros\7\default.xml .\%RUNTIMEDIR%\drivers\olethros\7\default.xml
if exist .\src\drivers\olethros\7\car6-trb1.rgb copy .\src\drivers\olethros\7\car6-trb1.rgb .\%RUNTIMEDIR%\drivers\olethros\7\car6-trb1.rgb

if exist .\src\drivers\olethros\8\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\8\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\8
if exist .\src\drivers\olethros\8\default.xml copy .\src\drivers\olethros\8\default.xml .\%RUNTIMEDIR%\drivers\olethros\8\default.xml
if exist .\src\drivers\olethros\8\car7-trb1.rgb copy .\src\drivers\olethros\8\car7-trb1.rgb .\%RUNTIMEDIR%\drivers\olethros\8\car7-trb1.rgb

if exist .\src\drivers\olethros\9\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\9\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros\9
if exist .\src\drivers\olethros\9\default.xml copy .\src\drivers\olethros\9\default.xml .\%RUNTIMEDIR%\drivers\olethros\9\default.xml
if exist .\src\drivers\olethros\9\car1-trb3.rgb copy .\src\drivers\olethros\9\car1-trb3.rgb .\%RUNTIMEDIR%\drivers\olethros\9\car1-trb3.rgb

if exist .\src\drivers\olethros\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\drivers\olethros\*.* call .\create_dir .\%RUNTIMEDIR%\drivers
if exist .\src\drivers\olethros\*.* call .\create_dir .\%RUNTIMEDIR%\drivers\olethros
if exist .\src\drivers\olethros\olethros.xml copy .\src\drivers\olethros\olethros.xml .\%RUNTIMEDIR%\drivers\olethros\olethros.xml
if exist .\src\drivers\olethros\logo.rgb copy .\src\drivers\olethros\logo.rgb .\%RUNTIMEDIR%\drivers\olethros\logo.rgb


if exist .\src\libs\raceengineclient\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\libs\raceengineclient\*.* call .\create_dir .\%RUNTIMEDIR%\config
if exist .\src\libs\raceengineclient\raceengine.xml copy .\src\libs\raceengineclient\raceengine.xml .\%RUNTIMEDIR%\config\raceengine.xml
if exist .\src\libs\raceengineclient\style.xsl copy .\src\libs\raceengineclient\style.xsl .\%RUNTIMEDIR%\config\style.xsl

if exist .\src\libs\tgf\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\libs\tgf\*.* call .\create_dir .\%RUNTIMEDIR%\config
if exist .\src\libs\tgf\params.dtd copy .\src\libs\tgf\params.dtd .\%RUNTIMEDIR%\config\params.dtd

if exist .\src\libs\tgfclient\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\libs\tgfclient\*.* call .\create_dir .\%RUNTIMEDIR%\config
if exist .\src\libs\tgfclient\screen.xml copy .\src\libs\tgfclient\screen.xml .\%RUNTIMEDIR%\config\screen.xml

if exist .\src\modules\graphic\ssggraph\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\modules\graphic\ssggraph\*.* call .\create_dir .\%RUNTIMEDIR%\config
if exist .\src\modules\graphic\ssggraph\graph.xml copy .\src\modules\graphic\ssggraph\graph.xml .\%RUNTIMEDIR%\config\graph.xml
if exist .\src\modules\graphic\ssggraph\sound.xml copy .\src\modules\graphic\ssggraph\sound.xml .\%RUNTIMEDIR%\config\sound.xml

if exist .\src\modules\telemetry\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\modules\telemetry\*.* call .\create_dir .\%RUNTIMEDIR%\telemetry
if exist .\src\modules\telemetry\telemetry.sh copy .\src\modules\telemetry\telemetry.sh .\%RUNTIMEDIR%\telemetry\telemetry.sh

if exist .\src\raceman\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\src\raceman\*.* call .\create_dir .\%RUNTIMEDIR%\config
if exist .\src\raceman\*.* call .\create_dir .\%RUNTIMEDIR%\config\raceman
if exist .\src\raceman\champ.xml copy .\src\raceman\champ.xml .\%RUNTIMEDIR%\config\raceman\champ.xml
if exist .\src\raceman\dtmrace.xml copy .\src\raceman\dtmrace.xml .\%RUNTIMEDIR%\config\raceman\dtmrace.xml
if exist .\src\raceman\endrace.xml copy .\src\raceman\endrace.xml .\%RUNTIMEDIR%\config\raceman\endrace.xml
if exist .\src\raceman\ncrace.xml copy .\src\raceman\ncrace.xml .\%RUNTIMEDIR%\config\raceman\ncrace.xml
if exist .\src\raceman\practice.xml copy .\src\raceman\practice.xml .\%RUNTIMEDIR%\config\raceman\practice.xml
if exist .\src\raceman\quickrace.xml copy .\src\raceman\quickrace.xml .\%RUNTIMEDIR%\config\raceman\quickrace.xml

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\results
call .\create_dir .\%RUNTIMEDIR%\results\champ

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\results
call .\create_dir .\%RUNTIMEDIR%\results\dtmrace

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\results
call .\create_dir .\%RUNTIMEDIR%\results\endrace

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\results
call .\create_dir .\%RUNTIMEDIR%\results\ncrace

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\results
call .\create_dir .\%RUNTIMEDIR%\results\practice

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\results
call .\create_dir .\%RUNTIMEDIR%\results\quickrace

if exist .\*.* call .\create_dir .\%RUNTIMEDIR%
if exist .\*.* call .\create_dir .\%RUNTIMEDIR%\.
if exist .\setup_linux.sh copy .\setup_linux.sh .\%RUNTIMEDIR%\.\setup_linux.sh

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\berniw

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\berniw2

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\berniw3

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\bt

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\damned

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\human

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\inferno

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\inferno2

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\lliaw

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\sparkle

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\drivers
call .\create_dir .\%RUNTIMEDIR%\drivers\tita

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\.

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\.

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\.

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\.

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\.

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\.

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\.

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\.

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\modules
call .\create_dir .\%RUNTIMEDIR%\modules\graphic

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\modules
call .\create_dir .\%RUNTIMEDIR%\modules\simu

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\modules
call .\create_dir .\%RUNTIMEDIR%\modules\simu

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\modules
call .\create_dir .\%RUNTIMEDIR%\modules\telemetry

call .\create_dir .\%RUNTIMEDIR%
call .\create_dir .\%RUNTIMEDIR%\modules
call .\create_dir .\%RUNTIMEDIR%\modules\track
