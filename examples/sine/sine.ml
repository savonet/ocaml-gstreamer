open Gstreamer

let () =
  init ();
  Printf.printf "%s\n%!" (version_string ());
  let bin = Pipeline.parse_launch "audiotestsrc ! audioconvert ! audioresample ! osssink" in
    ignore (Element.set_state bin State_playing);
    Unix.sleep 5;
    ignore (Element.set_state bin State_null);
    Gc.full_major ()
