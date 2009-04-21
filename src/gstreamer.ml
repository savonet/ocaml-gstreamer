include Gstreamer_idl

exception Null_pointer

let () =
  Callback.register_exception "gst_exn_null_pointer" Null_pointer

let init ?argv () = gst_init (match argv with None -> 0 | Some argv -> Array.length argv) argv
let version = gst_version
let version_string = gst_version_string

module Element =
struct
  type t = gstElement

  let set_property = set_element_property

  let link src dst = ignore (gst_element_link src dst) (* TODO: raise *)

  let link_many e =
    let e = Array.of_list e in
      for i = 0 to Array.length e - 2 do
        link e.(i) e.(i+1)
      done

  let set_state = gst_element_set_state
end

module Element_factory =
struct
  let make = gst_element_factory_make
end

module Pipeline =
struct
  let create = gst_pipeline_new
end

module Bin =
struct
  type t = gstBin

  let of_element = gst_bin_of_element

  let add b e = ignore (gst_bin_add b e) (* TODO: raise *)

  let add_many b e =
    List.iter (add b) e
end
