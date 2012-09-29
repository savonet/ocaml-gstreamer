open Gstreamer

let () =
  init ();
  Printf.printf "%s\n%!" (version_string ());
  let bin = Pipeline.create "pipeline" in
  let filesrc = Element_factory.make "filesrc" "disk_source" in
  Element.set_property_string filesrc "location" Sys.argv.(1);
  let decoder = Element_factory.make "mad" "decode" in
  let conv = Element_factory.make "audioconvert" "audioconvert" in
  let resample = Element_factory.make "audioresample" "audioresample" in
  let audiosink = Element_factory.make "pulsesink" "play_audio" in
    Bin.add_many (Bin.of_element bin) [filesrc; decoder; conv; resample; audiosink];
    Element.link_many [filesrc; decoder; conv; resample; audiosink];
    ignore (Element.set_state bin State_playing);
    Unix.sleep 5;
    ignore (Element.set_state bin State_null);
    Gc.full_major ()
