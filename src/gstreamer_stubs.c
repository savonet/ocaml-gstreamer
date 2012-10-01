#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/bigarray.h>
#include <caml/camlidlruntime.h>
#include <caml/threads.h>

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <gst/gst.h>
#include <gst/gstclock.h>
#include <gst/gsttypefind.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

CAMLprim value ocaml_gstreamer_init(value _argv)
{
  CAMLparam1(_argv);
  char **argv = NULL;
  int argc = 0;
  int len, i;

  if (Is_block(_argv))
    {
      _argv = Field(_argv, 0);
      argc = Wosize_val(_argv);
      argv = malloc(argc*sizeof(char*));
      for(i = 0; i < argc; i++)
        {
          len = caml_string_length(Field(_argv,i));
          argv[i] = malloc(len+1);
          memcpy(argv[i], String_val(Field(_argv,i)), len+1);
        }
    }

  caml_release_runtime_system();
  gst_init(&argc, &argv);
  for(i = 0; i < argc; i++)
    free(argv[i]);
  free(argv);
  caml_acquire_runtime_system();

  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_deinit(value unit)
{
  CAMLparam0();

  caml_release_runtime_system();
  gst_deinit();
  caml_acquire_runtime_system();

  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_version(value unit)
{
  CAMLparam0();
  CAMLlocal1(ans);

  unsigned int major, minor, micro, nano;
  gst_version(&major, &minor, &micro, &nano);

  ans = caml_alloc_tuple(4);
  Store_field(ans,0,Val_int(major));
  Store_field(ans,1,Val_int(minor));
  Store_field(ans,2,Val_int(micro));
  Store_field(ans,3,Val_int(nano));

  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_version_string(value unit)
{
  CAMLparam0();
  CAMLreturn(caml_copy_string(gst_version_string()));
}

/***** Element ******/

#define Element_val(v) (*(GstElement**)Data_custom_val(v))

static void finalize_element(value v)
{
  GstElement *e = Element_val(v);
  gst_object_unref(e);
}

static struct custom_operations element_ops =
  {
    "ocaml_gstreamer_element",
    finalize_element,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
  };

static value value_of_element(GstElement *e)
{
  if (!e) caml_raise_constant(*caml_named_value("gstreamer_exn_error"));
  value ans = caml_alloc_custom(&element_ops, sizeof(GstElement*), 0, 1);
  Element_val(ans) = e;
  return ans;
}

CAMLprim value ocaml_gstreamer_element_set_property_string(value e, value l, value v)
{
  CAMLparam3(e,l,v);
  g_object_set (Element_val(e), String_val(l), String_val(v), NULL);
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_element_set_property_int(value e, value l, value v)
{
  CAMLparam3(e,l,v);
  g_object_set (Element_val(e), String_val(l), Int_val(v), NULL);
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_element_set_property_bool(value e, value l, value v)
{
  CAMLparam3(e,l,v);
  g_object_set (Element_val(e), String_val(l), Bool_val(v), NULL);
  CAMLreturn(Val_unit);
}

static GstState state_of_val(value v)
{
  switch(Int_val(v))
    {
    case 0:
      return GST_STATE_VOID_PENDING;

    case 1:
      return GST_STATE_NULL;

    case 2:
      return GST_STATE_READY;

    case 3:
      return GST_STATE_PAUSED;

    case 4:
      return GST_STATE_PLAYING;

    default:
      assert(0);
    }
}

static value value_of_state_change_return(GstStateChangeReturn ret)
{
  switch(ret)
    {
    case GST_STATE_CHANGE_FAILURE:
      /* TODO: raise an error */
      assert(0);

    case GST_STATE_CHANGE_SUCCESS:
      return Val_int(0);

    case GST_STATE_CHANGE_ASYNC:
      return Val_int(1);

    case GST_STATE_CHANGE_NO_PREROLL:
      return Val_int(2);

    default:
      assert(0);
    }
}

CAMLprim value ocaml_gstreamer_element_set_state(value _e, value _s)
{
  CAMLparam2(_e, _s);
  GstElement *e = Element_val(_e);
  GstState s = state_of_val(_s);
  GstStateChangeReturn ret;

  caml_release_runtime_system();
  ret = gst_element_set_state(e, s);
  caml_acquire_runtime_system();

  CAMLreturn(value_of_state_change_return(ret));
}

CAMLprim value ocaml_gstreamer_element_get_state(value _e)
{
  CAMLparam1(_e);
  CAMLlocal1(ans);
  GstElement *e = Element_val(_e);
  GstStateChangeReturn ret;
  GstState state, pending;
  GstClockTime timeout = GST_CLOCK_TIME_NONE; /* TODO */

  caml_release_runtime_system();
  ret = gst_element_get_state(e, &state, &pending, timeout);
  caml_acquire_runtime_system();

  ans = caml_alloc_tuple(3);
  Store_field(ans, 0, value_of_state_change_return(ret));
  Store_field(ans, 1, state_of_val(state));
  Store_field(ans, 2, state_of_val(pending));
  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_element_link(value _src, value _dst)
{
  CAMLparam2(_src, _dst);
  GstElement *src = Element_val(_src);
  GstElement *dst = Element_val(_dst);
  gboolean ret;

  caml_release_runtime_system();
  ret = gst_element_link(src, dst);
  caml_acquire_runtime_system();

  assert(ret);
  CAMLreturn(Val_unit);
}

/***** Element factory *****/

CAMLprim value ocaml_gstreamer_element_factory_make(value factname, value name)
{
  CAMLparam2(factname, name);
  CAMLlocal1(ans);
  GstElement *e;

  e = gst_element_factory_make(String_val(factname), String_val(name));
  ans = value_of_element(e);

  CAMLreturn(ans);
}

/**** Message *****/

#define message_types_len 7
static const GstMessageType message_types[message_types_len] = { GST_MESSAGE_ERROR, GST_MESSAGE_TAG, GST_MESSAGE_STATE_CHANGED, GST_MESSAGE_STREAM_STATUS, GST_MESSAGE_DURATION_CHANGED, GST_MESSAGE_ASYNC_DONE, GST_MESSAGE_STREAM_START };

static GstMessageType message_type_of_int(int n)
{
  return message_types[n];
}

static int int_of_message_type(GstMessageType msg)
{
  int i;
  for (i = 0; i < message_types_len; i++)
    {
      if (msg == message_types[i])
        return i;
    }
  printf("error in message: %d\n", msg);
  assert(0);
}

#define Message_val(v) (*(GstMessage**)Data_custom_val(v))

static void finalize_message(value v)
{
  GstMessage *e = Message_val(v);
  gst_message_unref(e);
}

static struct custom_operations message_ops =
  {
    "ocaml_gstreamer_message",
    finalize_message,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
  };

static value value_of_message(GstMessage *msg)
{
  if (!msg) caml_raise_constant(*caml_named_value("gstreamer_exn_error"));
  value ans = caml_alloc_custom(&message_ops, sizeof(GstMessage*), 0, 1);
  Message_val(ans) = msg;
  return ans;
}

CAMLprim value ocaml_gstreamer_message_type(value _msg)
{
  CAMLparam1(_msg);
  GstMessage *msg = Message_val(_msg);
  CAMLreturn(Val_int(int_of_message_type(GST_MESSAGE_TYPE(msg))));
}

CAMLprim value ocaml_gstreamer_message_source_name(value _msg)
{
  CAMLparam1(_msg);
  GstMessage *msg = Message_val(_msg);
  CAMLreturn(caml_copy_string(GST_MESSAGE_SRC_NAME(msg)));
}

CAMLprim value ocaml_gstreamer_message_parse_tag(value _msg)
{
  CAMLparam1(_msg);
  CAMLlocal4(v,s,t,ans);
  GstMessage *msg = Message_val(_msg);
  GstTagList *tags = NULL;
  const GValue *val;
  const gchar *tag;
  int taglen;
  int i, j, n;

  caml_release_runtime_system();
  gst_message_parse_tag(msg, &tags);
  taglen = gst_tag_list_n_tags(tags);
  caml_acquire_runtime_system();

  ans = caml_alloc_tuple(taglen);
  for(i = 0; i < taglen; i++)
    {
      t = caml_alloc_tuple(2);

      // Tag name
      tag = gst_tag_list_nth_tag_name(tags, i);
      Store_field(t, 0, caml_copy_string(tag));

      // Tag fields
      n = gst_tag_list_get_tag_size(tags, tag);
      v = caml_alloc_tuple(n);
      for (j = 0; j < n; j++)
        {
          val = gst_tag_list_get_value_index(tags, tag, j);
          if (G_VALUE_HOLDS_STRING(val))
              s = caml_copy_string(g_value_get_string(val));
          else
            {
              //TODO: better typed handling of non-string values?
              char *vc = g_strdup_value_contents(val);
              s = caml_copy_string(vc);
              free(vc);
            }
          Store_field(v, j, s);
        }
      Store_field(t, 1, v);

      Store_field(ans, i, t);
    }

  gst_tag_list_unref(tags);

  CAMLreturn(ans);
}

/**** Bus ****/

typedef struct {
  GstBus *bus;
  value element;
} bus_t;

#define Bus_data_val(v) (*(bus_t**)Data_custom_val(v))
#define Bus_val(v) (Bus_data_val(v)->bus)

static void finalize_bus(value v)
{
  bus_t *bus = Bus_data_val(v);
  caml_remove_global_root(&bus->element);
  free(bus);
}

static struct custom_operations bus_ops =
  {
    "ocaml_gstreamer_bus",
    finalize_bus,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
  };

static value value_of_bus(GstBus *b)
{
  if (!b) caml_raise_constant(*caml_named_value("gstreamer_exn_error"));
  value ans = caml_alloc_custom(&bus_ops, sizeof(bus_t*), 0, 1);
  bus_t *bus = malloc(sizeof(bus));
  bus->bus = b;
  bus->element = 0;
  caml_register_global_root(&bus->element);
  Bus_data_val(ans) = bus;
  return ans;
}

CAMLprim value ocaml_gstreamer_bus_of_element(value _e)
{
  CAMLparam1(_e);
  CAMLlocal1(ans);
  GstElement *e = Element_val(_e);
  ans = value_of_bus(GST_ELEMENT_BUS(e));
  /* We keep a ref on this element so that the bus does not get invalidated by
     the garbage collection of e */
  Bus_data_val(ans)->element = _e;
  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_bus_pop_filtered(value _bus, value _filter)
{
  CAMLparam2(_bus, _filter);
  CAMLlocal1(ans);
  GstBus *bus = Bus_val(_bus);
  GstMessageType filter = 0;
  GstMessage *msg;
  int i;

  for(i = 0; i < Wosize_val(_filter); i++)
    filter |= message_type_of_int(Field(_filter, i));

  caml_release_runtime_system();
  msg = gst_bus_pop_filtered(bus, filter);
  caml_acquire_runtime_system();

  if(!msg)
    ans = Val_int(0);
  else
    {
      ans = caml_alloc_tuple(1);
      Store_field(ans, 0, value_of_message(msg));
    }

  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_bus_timed_pop_filtered(value _bus, value _filter)
{
  CAMLparam2(_bus, _filter);
  CAMLlocal1(ans);
  GstBus *bus = Bus_val(_bus);
  GstMessageType filter = 0;
  GstMessage *msg;
  int i;

  for(i = 0; i < Wosize_val(_filter); i++)
    filter |= message_type_of_int(Field(_filter, i));

  caml_release_runtime_system();
  msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, filter);
  caml_acquire_runtime_system();

  /* TODO: raise a timeout exception */
  assert(msg);

  CAMLreturn(value_of_message(msg));
}

/***** Bin ******/

#define Bin_val(v) GST_BIN(Element_val(v))

CAMLprim value ocaml_gstreamer_bin_add(value _bin, value _e)
{
  CAMLparam2(_bin, _e);
  GstBin *bin = Bin_val(_bin);
  GstElement *e = Element_val(_e);
  gboolean ret;

  caml_release_runtime_system();
  /* TODO: we have to ref here and have the bin unref it children at finalize!
     Namely, the bin should not have references to deallocated childs. */
  //gst_object_ref(e);
  ret = gst_bin_add(bin, e);
  caml_acquire_runtime_system();

  assert(ret);

  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_bin_get_by_name(value _bin, value _name)
{
  CAMLparam2(_bin, _name);
  CAMLlocal1(ans);
  GstBin *bin = Bin_val(_bin);
  GstElement *e;

  e = gst_bin_get_by_name(bin, String_val(_name));

  CAMLreturn(value_of_element(e));
}

/***** Pipeline *****/

#define Pipeline_val(v) GST_PIPELINE(Element_val(v))

CAMLprim value ocaml_gstreamer_pipeline_create(value s)
{
  CAMLparam1(s);
  CAMLlocal1(ans);
  GstElement *e;

  e = gst_pipeline_new(String_val(s));

  ans = value_of_element(e);
  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_pipeline_parse_launch(value s)
{
  CAMLparam1(s);
  CAMLlocal1(ans);
  GError *err = NULL;
  GstElement *e;

  e = gst_parse_launch(String_val(s), &err);
  if (err != NULL)
    {
      value s = caml_copy_string(err->message);
      if (e) { gst_object_unref(e); }
      g_error_free(err);
      caml_raise_with_arg(*caml_named_value("gst_exn_gerror"), s);
    }
  ans = value_of_element(e);

  CAMLreturn(ans);
}

/***** Appsrc *****/

typedef struct {
  GstAppSrc *appsrc;
  value need_data_cb; // Callback function
  gulong need_data_hid; // Callback handler ID
} appsrc;

#define Appsrc_val(v) (*(appsrc**)Data_custom_val(v))

static void disconnect_need_data(appsrc *as)
{
  if(as->need_data_cb)
    {
      caml_remove_global_root(&as->need_data_cb);
      as->need_data_cb = 0;
    }
  if(as->need_data_hid)
    {
      g_signal_handler_disconnect(as->appsrc, as->need_data_hid);
      as->need_data_hid = 0;
    }
}

static void finalize_appsrc(value v)
{
  appsrc *as = Appsrc_val(v);
  disconnect_need_data(as);
  free(as);
}

static struct custom_operations appsrc_ops =
  {
    "ocaml_gstreamer_appsrc",
    finalize_appsrc,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
  };

static value value_of_appsrc(GstAppSrc *e)
{
  value ans = caml_alloc_custom(&appsrc_ops, sizeof(appsrc*), 0, 1);
  appsrc *as = malloc(sizeof(appsrc));
  as->appsrc = e;
  as->need_data_cb = 0;
  as->need_data_hid = 0;
  Appsrc_val(ans) = as;
  return ans;
}

CAMLprim value ocaml_gstreamer_appsrc_of_element(value _e)
{
  CAMLparam1(_e);
  GstElement *e = Element_val(_e);
  CAMLreturn(value_of_appsrc(GST_APP_SRC(e)));
}

CAMLprim value ocaml_gstreamer_appsrc_to_element(value _as)
{
  CAMLparam1(_as);
  appsrc *as = Appsrc_val(_as);
  CAMLreturn(value_of_element(GST_ELEMENT(as->appsrc)));
}

CAMLprim value ocaml_gstreamer_appsrc_push_buffer_string(value _as, value _buf)
{
  CAMLparam2(_as, _buf);
  int buflen = caml_string_length(_buf);
  appsrc *as = Appsrc_val(_as);
  GstBuffer *gstbuf;
  GstMapInfo map;
  GstFlowReturn ret;
  gboolean bret;

  caml_release_runtime_system();
  gstbuf = gst_buffer_new_and_alloc(buflen);
  bret = gst_buffer_map (gstbuf, &map, GST_MAP_WRITE);
  caml_acquire_runtime_system();

  assert(bret);
  memcpy(map.data, (unsigned char*)String_val(_buf), buflen);

  caml_release_runtime_system();
  gst_buffer_unmap(gstbuf, &map);
  ret = gst_app_src_push_buffer(as->appsrc, gstbuf);
  caml_acquire_runtime_system();

  //TODO: raise
  assert(ret == GST_FLOW_OK);

  CAMLreturn(Val_unit);
}

static void appsrc_need_data_cb(GstAppSrc *gas, guint length, gpointer user_data)
{
  appsrc *as = (appsrc*)user_data;

  caml_c_thread_register();
  caml_acquire_runtime_system();
  caml_callback(as->need_data_cb, Val_int(length));
  caml_release_runtime_system();
  caml_c_thread_unregister();
}

CAMLprim value ocaml_gstreamer_appsrc_connect_need_data(value _as, value f)
{
  CAMLparam2(_as, f);
  appsrc *as = Appsrc_val(_as);
  disconnect_need_data(as);

  caml_register_global_root(&as->need_data_cb);

  caml_release_runtime_system();
  as->need_data_cb = f;
  as->need_data_hid = g_signal_connect(as->appsrc, "need-data", G_CALLBACK(appsrc_need_data_cb), as);
  caml_acquire_runtime_system();

  assert(as->need_data_hid);
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_appsrc_end_of_stream(value _as)
{
  CAMLparam1(_as);
  appsrc *as = Appsrc_val(_as);
  GstFlowReturn ret;

  caml_release_runtime_system();
  g_signal_emit_by_name(as->appsrc, "end-of-stream", &ret);
  caml_acquire_runtime_system();

  //TODO: raise
  assert(ret == GST_FLOW_OK);

  CAMLreturn(Val_unit);
}

/***** Appsink *****/

typedef struct {
  GstAppSink *appsink;
  value new_buffer_cb; // Callback function
  gulong new_buffer_hid; // Callback handler ID
} appsink;

#define Appsink_val(v) (*(appsink**)Data_custom_val(v))

static void disconnect_new_buffer(appsink *as)
{
  if(as->new_buffer_cb)
    {
      caml_remove_global_root(&as->new_buffer_cb);
      as->new_buffer_cb = 0;
    }
  if(as->new_buffer_hid)
    {
      g_signal_handler_disconnect(as->appsink, as->new_buffer_hid);
      as->new_buffer_hid = 0;
    }
}

static void finalize_appsink(value v)
{
  appsink *as = Appsink_val(v);
  disconnect_new_buffer(as);
  free(as);
}

static struct custom_operations appsink_ops =
  {
    "ocaml_gstreamer_appsink",
    finalize_appsink,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
  };

static value value_of_appsink(GstAppSink *e)
{
  value ans = caml_alloc_custom(&appsink_ops, sizeof(appsink*), 0, 1);
  appsink *as = malloc(sizeof(appsink));
  as->appsink = e;
  as->new_buffer_cb = 0;
  as->new_buffer_hid = 0;
  Appsink_val(ans) = as;
  return ans;
}

CAMLprim value ocaml_gstreamer_appsink_of_element(value _e)
{
  CAMLparam1(_e);
  GstElement *e = Element_val(_e);
  CAMLreturn(value_of_appsink(GST_APP_SINK(e)));
}

CAMLprim value ocaml_gstreamer_appsink_emit_signals(value _as)
{
  CAMLparam0();
  appsink *as = Appsink_val(_as);

  caml_release_runtime_system();
  gst_app_sink_set_emit_signals(as->appsink, TRUE);
  caml_acquire_runtime_system();

  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_appsink_pull_buffer(value _as)
{
  CAMLparam1(_as);
  CAMLlocal1(ans);
  appsink *as = Appsink_val(_as);
  GstSample *gstsample;
  GstBuffer *gstbuf;
  GstMapInfo map;
  intnat len;
  gboolean ret;

  caml_release_runtime_system();
  gstsample = gst_app_sink_pull_sample(as->appsink);
  caml_acquire_runtime_system();

  if (!gstsample)
    {
      if (gst_app_sink_is_eos(as->appsink))
        caml_raise_constant(*caml_named_value("gstreamer_exn_eos"));
      else
        caml_raise_constant(*caml_named_value("gstreamer_exn_error"));
    }

  caml_release_runtime_system();
  gstbuf = gst_sample_get_buffer(gstsample);
  caml_acquire_runtime_system();

  assert(gstbuf);

  caml_release_runtime_system();
  ret = gst_buffer_map(gstbuf, &map, GST_MAP_READ);
  caml_acquire_runtime_system();

  assert(ret);

  len = map.size;
  ans = caml_ba_alloc(CAML_BA_C_LAYOUT | CAML_BA_UINT8, 1, NULL, &len);
  memcpy(Caml_ba_data_val(ans), map.data, len);

  caml_release_runtime_system();
  gst_buffer_unmap(gstbuf, &map);
  gst_sample_unref(gstsample);
  caml_acquire_runtime_system();

  CAMLreturn(ans);
}

/* TODO: optimized version? */
CAMLprim value ocaml_gstreamer_appsink_pull_buffer_string(value _as)
{
  CAMLparam1(_as);
  CAMLlocal2(ba,ans);
  int len;

  ba = ocaml_gstreamer_appsink_pull_buffer(_as);
  len = Caml_ba_array_val(ba)->dim[0];
  ans = caml_alloc_string(len);
  memcpy(String_val(ans), Caml_ba_data_val(ba), len);

  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_appsink_is_eos(value _as)
{
  CAMLparam1(_as);
  appsink *as = Appsink_val(_as);
  gboolean ret;

  caml_release_runtime_system();
  ret = gst_app_sink_is_eos(as->appsink);
  caml_acquire_runtime_system();

  CAMLreturn(Val_bool(ret));
}

static GstFlowReturn appsink_new_buffer_cb(GstAppSink *gas, gpointer user_data)
{
  appsink *as = (appsink*)user_data;

  caml_c_thread_register();
  caml_acquire_runtime_system();
  caml_callback(as->new_buffer_cb, Val_unit);
  caml_release_runtime_system();
  caml_c_thread_unregister();

  return GST_FLOW_OK;
}

CAMLprim value ocaml_gstreamer_appsink_connect_new_buffer(value _as, value f)
{
  CAMLparam2(_as, f);
  appsink *as = Appsink_val(_as);
  disconnect_new_buffer(as);

  caml_register_global_root(&as->new_buffer_cb);

  caml_release_runtime_system();
  as->new_buffer_cb = f;
  as->new_buffer_hid = g_signal_connect(as->appsink, "new-buffer", G_CALLBACK(appsink_new_buffer_cb), as);
  caml_acquire_runtime_system();

  assert(as->new_buffer_hid);
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_appsink_set_max_buffers(value _as, value _n)
{
  CAMLparam2(_as, _n);
  appsink *as = Appsink_val(_as);
  int n = Int_val(_n);

  caml_release_runtime_system();
  gst_app_sink_set_max_buffers(as->appsink, n);
  caml_acquire_runtime_system();

  CAMLreturn(Val_unit);
}

/***** Typefind *****/

typedef struct {
  GstTypeFind *tf;
  value have_type_cb; // Callback function
  gulong have_type_hid; // Callback handler ID
} typefind;

#define Typefind_data_val(v) (*(typefind**)Data_custom_val(v))
#define Typefind_val(v) (Typefind_data_val(v)->tf)

static void disconnect_have_type(typefind *tf)
{
  if(tf->have_type_cb)
    {
      caml_remove_global_root(&tf->have_type_cb);
      tf->have_type_cb = 0;
    }
  if(tf->have_type_hid)
    {
      g_signal_handler_disconnect(tf->tf, tf->have_type_hid);
      tf->have_type_hid = 0;
    }
}

static void finalize_typefind(value v)
{
  typefind *tf = Typefind_data_val(v);
  disconnect_have_type(tf);
  free(tf);
}

static struct custom_operations typefind_ops =
  {
    "ocaml_gstreamer_typefind",
    finalize_typefind,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
  };

static value value_of_typefind(GstTypeFind *e)
{
  value ans = caml_alloc_custom(&typefind_ops, sizeof(typefind*), 0, 1);
  typefind *tf = malloc(sizeof(typefind));
  tf->tf = e;
  tf->have_type_cb = 0;
  tf->have_type_hid = 0;
  Typefind_data_val(ans) = tf;
  return ans;
}

#define GST_TYPE_FIND(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TYPE_FIND,GstTypeFind))

CAMLprim value ocaml_gstreamer_typefind_of_element(value _e)
{
  CAMLparam1(_e);
  GstElement *e = Element_val(_e);
  CAMLreturn(value_of_typefind(GST_TYPE_FIND(e)));
}

static void typefind_have_type_cb(GstTypeFind *_typefind, guint probability, GstCaps *caps, gpointer user_data)
{
  typefind *tf = (typefind*)user_data;

  caml_c_thread_register();
  caml_acquire_runtime_system();
  //TODO: arguments: proba and caps!
  caml_callback(tf->have_type_cb, Val_unit);
  caml_release_runtime_system();
  caml_c_thread_unregister();
}

CAMLprim value ocaml_gstreamer_typefind_connect_have_type(value _tf, value f)
{
  CAMLparam2(_tf, f);
  typefind *tf = Typefind_data_val(_tf);
  disconnect_have_type(tf);

  caml_register_global_root(&tf->have_type_cb);

  caml_release_runtime_system();
  tf->have_type_cb = f;
  tf->have_type_hid = g_signal_connect(tf->tf, "have-type", G_CALLBACK(typefind_have_type_cb), tf);
  caml_acquire_runtime_system();

  assert(tf->have_type_hid);
  CAMLreturn(Val_unit);
}
