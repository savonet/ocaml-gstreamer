exception Null_pointer

val init : ?argv:string array -> unit -> unit

val version : unit -> int * int * int * int

val version_string : unit -> string

type state =
    State_void_pending
  | State_null
  | State_ready
  | State_paused
  | State_playing

module Element :
sig
  type t

  val set_property_string : t -> string -> string -> unit

  val set_property_bool : t -> string -> bool -> unit

  val set_property_int : t -> string -> int -> unit

    (* val set_caps : t -> pGstCaps -> unit *)

  val link : t -> t -> unit

  val link_many : t list -> unit

  val set_state : t -> state -> unit
end

module Element_factory :
sig
  val make : string -> string -> Element.t
end

module Pipeline :
sig
  val create : string -> Element.t

  val parse_launch : string -> Element.t
end

module Bin :
sig
  type t

  val of_element : Element.t -> t

  val add : t -> Element.t -> unit

  val add_many : t -> Element.t list -> unit

  val get_by_name : t -> string -> Element.t
end

module Caps :
sig
  type t

  val to_string : t -> string

  val of_string : string -> t
end

module App_sink :
sig
  type t

  val of_element : Element.t -> t

  val pull_buffer : t -> (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t

  val pull_buffer_string : t -> string
end
