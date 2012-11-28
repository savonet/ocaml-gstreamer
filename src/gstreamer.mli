exception Failure
exception Failure_msg of string
exception Timeout
exception End_of_stream

val init : ?argv:(string array) -> unit -> unit

val deinit : unit -> unit

val version : unit -> int * int * int * int

val version_string : unit -> string

type data = (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t

module Format : sig
  (** Format for durations. *)
  type t =
  | Undefined
  | Default
  | Bytes
  | Time (** time in nanoseconds *)
  | Buffers
  | Percent

  val to_string : t -> string
end

module Event : sig
  type seek_flag =
  | Seek_flag_none
  | Seek_flag_flush
  | Seek_flag_accurate
  | Seek_flag_key_unit
  | Seek_flag_segment
  | Seek_flag_skip
  | Seek_flag_snap_before
  | Seek_flag_snap_after
  | Seek_flag_snap_nearest
end

module Element : sig
  type t

  val set_property_string : t -> string -> string -> unit

  val set_property_int : t -> string -> string -> unit

  val set_property_bool : t -> string -> string -> unit

  type state =
  | State_void_pending
  | State_null
  | State_ready
  | State_paused
  | State_playing

  val string_of_state : state -> string

  type state_change =
  | State_change_sucess
  | State_change_async
  | State_change_no_preroll

  val set_state : t -> state -> state_change

  val get_state : t -> state_change * state * state

  val link : t -> t -> unit

  val link_many : t list -> unit

  val position : t -> Format.t -> Int64.t

  (** Seek to a given position relative to the start of the stream. *)
  val seek_simple : t -> Format.t -> Event.seek_flag list -> Int64.t -> unit
end

module Element_factory : sig
  type t = Element.t

  val make : string -> string -> t
end

module Message : sig
  type message_type =
  | Error
  | Tag
  | State_changed
  | Stream_status
  | Duration_changed
  | Async_done
  | Stream_start

  type t

  val message_type : t -> message_type

  val source_name : t -> string

  val parse_tag : t -> (string * string list) list
end

module Bus : sig
  type t

  val of_element : Element.t -> t

  val pop_filtered : t -> Message.message_type list -> Message.t option

  val timed_pop_filtered : t -> Message.message_type list -> Message.t
end

module Bin : sig
  type t = Element.t

  val of_element : Element.t -> t

  val add : t -> Element.t -> unit

  val add_many : t -> Element.t list -> unit

  (** [get_by_name "foo"] find a bin by name.
    * raises [Not_found] if element does not exist. *) 
  val get_by_name : t -> string -> Element.t
end

module Pipeline : sig
  type t = Element.t

  val create : string -> t

  val parse_launch : string -> t
end

module Buffer : sig
  type t

  val of_string : string -> int -> int -> t

  val of_data : data -> int -> int -> t

  val set_presentation_time : t -> Int64.t -> unit

  val set_decoding_time : t -> Int64.t -> unit

  val set_duration : t -> Int64.t -> unit
end

module App_src : sig
  type t

  val to_element : t -> Element.t

  val of_element : Element.t -> t

  val push_buffer : t -> Buffer.t -> unit

  val push_buffer_string : t -> string -> unit

  (** Register a callback that will be called when data need to be fed into the
      source (the argument is the number of bytes needed by the source). *)
  val on_need_data : t -> (int -> unit) -> unit

  (** Emit an end of stream signal. *)
  val end_of_stream : t -> unit

  val set_format : t -> Format.t -> unit
end

module App_sink : sig
  type t

  val of_element : Element.t -> t

  val pull_buffer_data : t -> data

  val pull_buffer_string : t -> string

  val emit_signals : t -> unit

  val is_eos : t -> bool

  val on_new_sample : t -> (unit -> unit) -> unit

  val set_max_buffers : t -> int -> unit
end

module Caps : sig
  type t

  val to_string : t -> string
end

module Type_find_element : sig
  type t

  val of_element : Element.t -> t

  val on_have_type : t -> (int -> Caps.t -> unit) -> unit
end

module Tag_setter : sig
  type t

  type merge_mode =
  | Undefined
  | Replace_all
  | Replace
  | Append
  | Prepend
  | Keep
  | Keep_all
  | Count

  val of_element : Element.t -> t

  val add_tag : t -> merge_mode -> string -> string -> unit
end
