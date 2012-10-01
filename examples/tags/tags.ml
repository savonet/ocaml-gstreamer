open Gstreamer

let () =
  Gstreamer.init ();
  let pipeline = Printf.sprintf "filesrc location=\"%s\" ! decodebin ! fakesink" Sys.argv.(1) in
  let bin = Pipeline.parse_launch pipeline in
  (* Go in paused state. *)
  ignore (Element.set_state bin State_paused);
  (* Wait for the state to complete. *)
  ignore (Element.get_state bin);
  try
    while true do
      let msg = Bus.pop_filtered (Bus.of_element bin) [Message.Error; Message.Tag; Message.Async_done] in
      let msg = match msg with Some msg -> msg | None -> raise Exit in
      Printf.printf "Message from %s\n%!" (Message.source_name msg);
      let typ = Message.message_type msg in
      if typ = Message.Error then failwith "Error message!";
      if typ = Message.Tag then
        let tags = Message.parse_tag msg in
        List.iter (fun (l,v) -> Printf.printf "- %s : %s\n%!" l (String.concat ", " v)) tags
    done
  with
  | Exit -> ()
