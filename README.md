# Multimedia_Proj_video_stream
Here we want to create a project which would be getting some input live feed from any camera broadcaster. Input stream will be recieved by the TS Integrator module which also collects some private data and multiplex those private data into the transport stream and sends it to the OpenGL/Private data integrator. OpenGL/Private data integrator module will collect the TS packets and pasres it to get the private data from it; then it uses respective module(OpenGL for animation,teletext for Subtitle, logo etc.) to integrate private or add data on to the live feed and distributes.
