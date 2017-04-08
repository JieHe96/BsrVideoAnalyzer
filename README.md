# BsrVideoAnalyzer
A simple video analyzing tool that could navigates video stream and displays macroblock(CTU for hevc) type in video playing window.

General Process: Using FFMpeg API to demux video stream => analyzing stream infomation => FFMpeg API doing decode => SDL displaying
		 => overlay macroblock or CTU info


Features:

		- UltraHD 4K video support.
		- Stream navigation and display in I, P, B, IP and Key Frames modes.
		- Display of reference frames.
		- Hex viewer.
		- Stream viewer – file content (header level) presentation in text mode.
		- Viewing of reference raw data.
		- Display and saving of stream summary and picture information.
		- Demux video stream and saving raw stream.
		- Saving yuv raw video and can select video frame range.
		- Display of bitstream headers with offsets and bit size indication (as in the Standard documentation) for all standards.
		- Supporting video format: mp4 avi mkv flv h264 hevc h265, automatically demux using ffmpeg lib
		- Display of Mackroblock / Coded Tree Unit: type quantizers
		- Navigation via thumbnails or button, step by step forward or backward.

PS: During the building process, if the Windows_SDK version does not fit the one in your computer, please try retarget the project, then rebuild to solve the issue.
