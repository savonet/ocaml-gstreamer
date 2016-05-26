open Gstreamer

let timeToString nsd =
  let msd = Int64.(to_int(div nsd (of_int 1000000))) in
  let h = msd / 3600000 in
  let m = (msd - (h * 3600000)) / 60000 in
  let s = (msd - (h * 3600000) - (m * 60000)) / 1000 in
  let ms = msd - (h * 3600000) - (m * 60000) - (s * 1000) in
  let h = if h = 0 then "" else Printf.sprintf "%d:" h in 
  let ms = if ms = 0 then "" else Printf.sprintf ".%d" ms in 
  Printf.sprintf "%s%d:%d%s" h m s ms

let printProgression bin =
  let position = timeToString(Element.position bin Format.Time) in
  let duration = timeToString(Element.duration bin Format.Time) in
  Printf.printf "Progression : %s / %s\n%!" position duration

let () =
  Gstreamer.init ();
  let pipeline = Printf.sprintf "filesrc location=\"%s\" ! decodebin ! fakesink" Sys.argv.(1) in
  let bin = Pipeline.parse_launch pipeline in

  ignore (Element.set_state bin Element.State_playing);
   (* Wait for the state to complete. *)
  ignore (Element.get_state bin);

  printProgression bin;

  Unix.sleep 1;
  
  printProgression bin;

  Unix.sleep 1;
  
  printProgression bin;

  Gc.full_major ()
