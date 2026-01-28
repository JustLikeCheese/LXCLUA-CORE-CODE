local bindClass = luajava.bindClass
local LinearLayoutCompat = bindClass "androidx.appcompat.widget.LinearLayoutCompat"
local AppCompatTextView = bindClass "androidx.appcompat.widget.AppCompatTextView"
local TextInputEditText = bindClass "com.google.android.material.textfield.TextInputEditText"
local TextInputLayout = bindClass "com.google.android.material.textfield.TextInputLayout"

return {
  LinearLayoutCompat,
  layout_width = -1,
  orientation = "vertical",
  padding = "16dp",
  {
    TextInputLayout,
    layout_width = -1,
    layout_marginBottom = "8dp",
    {
      TextInputEditText,
      id = "lsp_host",
      hint = "服务器地址",
      layout_width = -1,
      singleLine = true,
    },
  },
  {
    TextInputLayout,
    layout_width = -1,
    layout_marginBottom = "16dp",
    {
      TextInputEditText,
      id = "lsp_port",
      hint = "端口",
      layout_width = -1,
      inputType = "number",
      singleLine = true,
    },
  },
}