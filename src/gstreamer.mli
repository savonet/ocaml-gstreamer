val init : ?argv:(string array) -> unit -> unit

val version : unit -> int * int * int * int

val version_string : unit -> string

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

module Element : sig
  type t

  val set_property_string : t -> string -> string -> unit

  val set_property_int : t -> string -> string -> unit

  val set_property_bool : t -> string -> string -> unit

  val set_state : t -> state -> state_change
end

module Bin : sig
  type t = Element.t

  val get_by_name : t -> string -> Element.t
end

module Pipeline : sig
  type t = Element.t

  val parse_launch : string -> t
end

module App_src : sig
  type t

  val to_element : t -> Element.t

  val of_element : Element.t -> t

  val push_buffer_string : t -> string -> unit

  val connect_need_data : t -> (int -> unit) -> unit

  val end_of_stream : t -> unit
end

module App_sink : sig
  type t = Element.t

  val pull_buffer : t -> (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t
end
