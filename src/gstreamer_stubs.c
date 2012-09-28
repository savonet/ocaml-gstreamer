#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/bigarray.h>
#include <caml/camlidlruntime.h>

#include <string.h>
#include <assert.h>

#include <gst/gst.h>
#include <gst/gstclock.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

CAMLprim value ocaml_gstreamer_init(value _argv)
{
  CAMLparam1(_argv);

  char **argv = NULL;
  int argc = 0;
  if (Is_block(_argv))
    {
      //argc = Wosize_val(_argv);
      /* TODO */
    }
  gst_init(&argc, &argv);
  free(argv);

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

  ret = gst_element_set_state(e, s);

  CAMLreturn(value_of_state_change_return(ret));
}

/***** Bin ******/

#define Bin_val(v) GST_BIN(Element_val(v))

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

#define Appsrc_val(v) (GST_APP_SRC(Element_val(v)))

CAMLprim value ocaml_gstreamer_appsrc_push_buffer_string(value _as, value _buf)
{
  CAMLparam2(_as, _buf);
  int buflen = caml_string_length(_buf);
  GstAppSrc *as = Appsrc_val(_as);
  GstBuffer *gstbuf = gst_buffer_new_and_alloc(buflen);
  GstFlowReturn ret;

  memcpy(GST_BUFFER_DATA(gstbuf), String_val(_buf), buflen);
  caml_enter_blocking_section();
  ret = gst_app_src_push_buffer(as, gstbuf);
  caml_leave_blocking_section();
  //TODO: raise
  assert(ret == GST_FLOW_OK);

  CAMLreturn(Val_unit);
}

/***** Appsink *****/

#define Appsink_val(v) (GST_APP_SINK(Element_val(v)))

CAMLprim value ocaml_gstreamer_appsink_pull_buffer(value _as)
{
  CAMLparam1(_as);
  GstAppSink *as = Appsink_val(_as);
  GstBuffer *gstbuf;
  char *data;
  intnat len;

  caml_enter_blocking_section();
  gstbuf = gst_app_sink_pull_buffer(as);
  //TODO: raise exception
  assert(gstbuf);
  len = gstbuf->size;
  data = malloc(len);
  memcpy(data, gstbuf->data, len);
  gst_buffer_unref(gstbuf);
  caml_leave_blocking_section();

  value ba = caml_ba_alloc(CAML_BA_MANAGED | CAML_BA_C_LAYOUT | CAML_BA_UINT8, 1, data, &len);
  CAMLreturn(ba);
}
