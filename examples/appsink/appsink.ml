open Gstreamer

let () =
  init ();
  Printf.printf "%s\n%!" (version_string ());
  let bin = Pipeline.parse_launch "audiotestsrc ! decodebin ! audio/x-raw,format=S16LE ! appsink name=sink sync=False" in
  let sink = Bin.get_by_name bin "sink" in
  ignore (Element.set_state bin State_playing);
  while true do
    let _ = App_sink.pull_buffer (App_sink.of_element sink) in
    ()
  done;
  ignore (Element.set_state bin State_null);
  Gc.full_major ()
