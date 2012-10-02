exception Failure
exception Failure_msg of string
exception Timeout
exception End_of_stream

let () =
  Callback.register_exception "gstreamer_exn_failure" Failure;
  Callback.register_exception "gstreamer_exn_failure_msg" (Failure_msg "");
  Callback.register_exception "gstreamer_exn_timeout" Timeout;
  Callback.register_exception "gstreamer_exn_eos" End_of_stream

external init : (string array) option -> unit = "ocaml_gstreamer_init"
let init ?argv () = init argv

external deinit : unit -> unit = "ocaml_gstreamer_deinit"

external version : unit -> int * int * int * int = "ocaml_gstreamer_version"

external version_string : unit -> string = "ocaml_gstreamer_version_string"


module Element = struct
  type t

  external set_property_string : t -> string -> string -> unit = "ocaml_gstreamer_element_set_property_string"

  external set_property_int : t -> string -> string -> unit = "ocaml_gstreamer_element_set_property_int"

  external set_property_bool : t -> string -> string -> unit = "ocaml_gstreamer_element_set_property_bool"

  type state =
  | State_void_pending
  | State_null
  | State_ready
  | State_paused
  | State_playing

  external string_of_state : state -> string = "ocaml_gstreamer_element_string_of_state"

  type state_change =
  | State_change_sucess
  | State_change_async
  | State_change_no_preroll

  external set_state : t -> state -> state_change = "ocaml_gstreamer_element_set_state"

  external get_state : t -> state_change * state * state = "ocaml_gstreamer_element_get_state"

  external link : t -> t -> unit = "ocaml_gstreamer_element_link"

  let link_many ee =
    let e, ee = List.hd ee, List.tl ee in
    ignore (List.fold_left (fun e e' -> link e e'; e') e ee)
end

module Element_factory = struct
  type t = Element.t

  external make : string -> string -> t = "ocaml_gstreamer_element_factory_make"
end

module Message = struct
  (* TODO: add more... *)
  type message_type =
  | Error
  | Tag
  | State_changed
  | Stream_status
  | Duration_changed
  | Async_done
  | Stream_start

  type t

  external message_type : t -> message_type = "ocaml_gstreamer_message_type"

  external source_name : t -> string = "ocaml_gstreamer_message_source_name"

  external parse_tag : t -> (string * string array) array = "ocaml_gstreamer_message_parse_tag"
  let parse_tag msg =
    let tags = parse_tag msg in
    let tags = Array.map (fun (l,v) -> l, Array.to_list v) tags in
    Array.to_list tags
end

module Bus = struct
  type t

  external of_element : Element.t -> t = "ocaml_gstreamer_bus_of_element"

  external pop_filtered : t -> Message.message_type array -> Message.t option = "ocaml_gstreamer_bus_pop_filtered"
  let pop_filtered bus filter = pop_filtered bus (Array.of_list filter)

  external timed_pop_filtered : t -> Message.message_type array -> Message.t = "ocaml_gstreamer_bus_timed_pop_filtered"
  let timed_pop_filtered bus filter = timed_pop_filtered bus (Array.of_list filter)
end

module Bin = struct
  type t = Element.t

  let of_element e = e

  external add : t -> Element.t -> unit = "ocaml_gstreamer_bin_add"

  let add_many bin e =
    List.iter (add bin) e

  external get_by_name : t -> string -> Element.t = "ocaml_gstreamer_bin_get_by_name"
end

module Pipeline = struct
  type t = Element.t

  external create : string -> t = "ocaml_gstreamer_pipeline_create"

  external parse_launch : string -> t = "ocaml_gstreamer_pipeline_parse_launch"
end

module App_src = struct
  type t

  external of_element : Element.t -> t = "ocaml_gstreamer_appsrc_of_element"

  external to_element : t -> Element.t = "ocaml_gstreamer_appsrc_to_element"

  external push_buffer_string : t -> string -> unit = "ocaml_gstreamer_appsrc_push_buffer_string"

  external on_need_data : t -> (int -> unit) -> unit = "ocaml_gstreamer_appsrc_connect_need_data"

  external end_of_stream : t -> unit = "ocaml_gstreamer_appsrc_end_of_stream"
end

module App_sink = struct
  type t

  external of_element : Element.t -> t = "ocaml_gstreamer_appsink_of_element"

  external pull_buffer : t -> (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t = "ocaml_gstreamer_appsink_pull_buffer"

  external pull_buffer_string : t -> string = "ocaml_gstreamer_appsink_pull_buffer_string"

  external emit_signals : t -> unit = "ocaml_gstreamer_appsink_emit_signals"

  external is_eos : t -> bool = "ocaml_gstreamer_appsink_is_eos"

  external on_new_buffer : t -> (unit -> unit) -> unit = "ocaml_gstreamer_appsink_connect_new_buffer"

  external set_max_buffers : t -> int -> unit = "ocaml_gstreamer_appsink_set_max_buffers"
end

module Type_find = struct
  type t

  external of_element : Element.t -> t = "ocaml_gstreamer_typefind_of_element"
end
