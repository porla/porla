import React from "react";
import { Formik, Field, Form } from "formik";
import { trpc } from "../utils/trpc";

function Add() {
  const mutation = trpc.useMutation(["torrents.add"]);

  return (
    <div>
      <h1>Add torrent</h1>
      <Formik
        initialValues={{ name: "", email: "" }}
        onSubmit={async ({ name }) => {
          await mutation.mutateAsync({ name })
        }}
      >
        <Form>
          <Field name="name" type="text" />
          <Field name="email" type="email" />
          <button type="submit">Submit</button>
        </Form>
      </Formik>
    </div>
  );
}

export default Add;
