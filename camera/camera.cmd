################################################
## camera command script
## grammar of script:
##      BEGIN ${stc_name}
##      connect ${id}
##      config preview=${width}x${height} picture=${width}x${height} fr=${framerate}
##      preview
##      zoom ${ratio}
##      autofocus
##      delay ${ms}
##      capture
##      release
##      END
################################################
BEGIN back_camera_13M_basic
connect 0
config preview=1440x1080 picture=4160x3120 fr=30
preview
delay 1000
capture
delay 3000
release
END
