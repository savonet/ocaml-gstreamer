include Gstreamer_idl

exception Null_pointer

let () =
  Callback.register_exception "gst_exn_null_pointer" Null_pointer

let init ?argv () =
  ocaml_gst_init (match argv with None -> 0 | Some argv -> Array.length argv) argv

let version = gst_version
let version_string = gst_version_string

type state =
  | State_void_pending
  | State_null
  | State_ready
  | State_paused
  | State_playing

let gstState_of_state (s:state) : gstState = (Obj.magic s:gstState)

module Element =
struct
  type t = pGstElement

  let set_property_string = set_element_property_string
  let set_property_bool = set_element_property_bool
  let set_property_int = set_element_property_int

  let set_caps = set_element_caps

  let link src dst = ignore (gst_element_link src dst) (* TODO: raise *)

  let link_many e =
    let e = Array.of_list e in
      for i = 0 to Array.length e - 2 do
        link e.(i) e.(i+1)
      done

  let set_state e s =
    (* TODO: return value? *)
    ignore (gst_element_set_state e (gstState_of_state s))
end

module Element_factory =
struct
  let make = gst_element_factory_make
end

module Pipeline =
struct
  let create = gst_pipeline_new

  let parse_launch = parse_launch
end

module Bin =
struct
  type t = pGstBin

  let of_element = gst_bin_of_element

  let add b e = ignore (gst_bin_add b e) (* TODO: raise *)

  let add_many b e =
    List.iter (add b) e

  let get_by_name = gst_bin_get_by_name
end

module Caps =
struct
  type t = pGstCaps

  let to_string = gst_caps_to_string

  let of_string = gst_caps_from_string
end

module AppSink =
struct
  external pull_buffer : pGstElement -> (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t = "caml_app_sink_pull_buffer"
end
