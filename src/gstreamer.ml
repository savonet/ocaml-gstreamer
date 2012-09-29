external init : (string array) option -> unit = "ocaml_gstreamer_init"
let init ?argv () = init argv

external version : unit -> int * int * int * int = "ocaml_gstreamer_version"

external version_string : unit -> string = "ocaml_gstreamer_version_string"

type state =
| State_void_pending
| State_null
| State_ready
| State_paused
| State_playing

type state_change =
| State_change_sucess
| State_change_async
| State_change_no_preroll

module Element = struct
  type t

  external set_property_string : t -> string -> string -> unit = "ocaml_gstreamer_element_set_property_string"

  external set_property_int : t -> string -> string -> unit = "ocaml_gstreamer_element_set_property_int"

  external set_property_bool : t -> string -> string -> unit = "ocaml_gstreamer_element_set_property_bool"

  external set_state : t -> state -> state_change = "ocaml_gstreamer_element_set_state"
end

module Bin = struct
  type t = Element.t

  external get_by_name : t -> string -> Element.t = "ocaml_gstreamer_bin_get_by_name"
end

module Pipeline = struct
  type t = Element.t

  external parse_launch : string -> t = "ocaml_gstreamer_pipeline_parse_launch"
end

module App_src = struct
  type t

  external of_element : Element.t -> t = "ocaml_gstreamer_appsrc_of_element"

  external to_element : t -> Element.t = "ocaml_gstreamer_appsrc_to_element"

  external push_buffer_string : t -> string -> unit = "ocaml_gstreamer_appsrc_push_buffer_string"

  external connect_need_data : t -> (int -> unit) -> unit = "ocaml_gstreamer_appsrc_connect_need_data"

  external end_of_stream : t -> unit = "ocaml_gstreamer_appsrc_end_of_stream"
end

module App_sink = struct
  type t = Element.t

  let of_element e = e

  external pull_buffer : t -> (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t = "ocaml_gstreamer_appsink_pull_buffer"

  external pull_buffer_string : t -> string = "ocaml_gstreamer_appsink_pull_buffer_string"
end
