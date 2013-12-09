/** @jsx React.DOM */
(function () {
  var Chat = React.createClass({displayName: 'Chat',

    getInitialState: function () {
      return {
        message: "Enter a hostname",
        name: localStorage.lastUsedName,
        messages : JSON.parse(localStorage.chat_messages),
        host: localStorage.lastUsedHost || "",
        loading:false
      };
    },

    onSubmit: function () {
      var messageInput = this.refs.message.getDOMNode();
      this.state.messages.push({message: messageInput.value, author: this.state.name, origin: "self"});
      localStorage.chat_messages = JSON.stringify(this.state.messages);
      this.state.socket.send(this.state.name + "|" + messageInput.value);
      this.setState({}, function () {
        var list = this.refs.list.getDOMNode();
        list.scrollTop = list.scrollHeight;
      }.bind(this));
      messageInput.value = "";
      return false;
    },

    onEnterName: function (name) {
      localStorage.lastUsedName = name;
      this.setState({name : name});
      return false;
    },

    onEnterHost: function (host) {
      this.setState({host : host, loading: true}, function() {
        this.startSocket();
      });
      return false;
    },

    startSocket: function () {
      try {
        this.state.socket = new WebSocket("ws://" + this.state.host);
      } catch (e) {
        console.log(e);
        this.setState({loading: false, connected: false});
        return;
      }

      this.state.socket.onerror = function (event) {
        console.log(event);
        this.setState({loading: false, connected: false});
      }.bind(this);

      this.state.socket.onclose = function () {
        console.log("closed");
        this.setState({message: "Connection closed", messageColor: "red",
                       loading: false, connected: false});
      }.bind(this);

      this.state.socket.onopen = function (event) {
        console.log("connected");

        document.getElementById('title').classList.add('smaller');
        document.getElementById('main').classList.add('larger');
        document.getElementById('chat').classList.add('visible');
        document.getElementById('title-text-left').classList.add('hidden');
        document.getElementById('title-text-right').classList.add('hidden');
        document.getElementById('username-text').innerHTML = this.state.name;
        document.getElementById('host-text').innerHTML = this.state.host;
        document.getElementById('username').classList.add('visible');
        document.getElementById('host').classList.add('visible');

        this.state.socket.onmessage = function (event) {
          var parts = event.data.split("|");
          this.state.messages.push({message: parts[1], author: parts[0], origin: "ext"});
          localStorage.chat_messages = JSON.stringify(this.state.messages);
          this.setState({}, function () {
            var list = this.refs.list.getDOMNode();
            list.scrollTop = list.scrollHeight;
          }.bind(this));
        }.bind(this);

        this.setState({loading: false, connected: true});
        setTimeout(function() {
          this.refs.message.getDOMNode().focus();
          var list = this.refs.list.getDOMNode();
          list.scrollTop = list.scrollHeight;
        }.bind(this), 1500);

        localStorage.lastUsedHost = this.state.host;
      }.bind(this);

    },

    render: function () {
      if(this.state.loading){
        return React.DOM.i( {className:"icon ion-loading-c"});
      }

      if(!this.state.name){
        return (
          NameForm( {onSubmit:this.onEnterName} )
        );
      }

      if(!this.state.connected){
        return (
          HostForm( {value:this.state.host, onSubmit:this.onEnterHost} )
        );
      }

      var messages = this.state.messages.map(function (value) {
        return (
          Message( {key:value.message + Math.random(),
                   author:value.author,
                   message:value.message,
                   origin:value.origin} )
        )
      });

      return (
        React.DOM.div(null, 
          React.DOM.div( {ref:"list", className:"messageList"}, 
            messages
          ),
          React.DOM.form( {onSubmit:this.onSubmit, className:"pure-form pure-g message-form"}, 
            React.DOM.div( {className:"pure-u-5-6"}, 
              React.DOM.input( {placeholder:"Write a message", type:"text", ref:"message",
                     maxLength:"100", className:"message-input"} )
            ),
            React.DOM.div( {className:"pure-u-1-6"}, 
              React.DOM.button( {type:"submit", className:"message-submit pure-button"}, 
                React.DOM.i( {className:"ion-ios7-paperplane-outline"})
              )
            )
          )
        )
      );
    }
  });

  var Message = React.createClass({displayName: 'Message',
    render: function () {
      return (
        React.DOM.div( {className:"message " + this.props.origin}, 
          React.DOM.span( {className:"message-text"}, this.props.message),
          React.DOM.span( {className:"message-author"}, this.props.author)
        )
      )
    }
  });

  var NameForm = React.createClass({displayName: 'NameForm',
    componentDidMount: function () {
      this.refs.attr.getDOMNode().focus();
    },

    onSubmit: function () {
      return this.props.onSubmit(this.refs.attr.getDOMNode().value);
    },

    render : function () {
      return (
        React.DOM.form( {className:"attribute-form pure-form", onSubmit:this.onSubmit}, 
          React.DOM.h2(null, "Enter a name"),
          React.DOM.input( {type:"text", pattern:".{3,}", ref:"attr"} ),
          React.DOM.button( {className:"pure-button", type:"submit"}, 
            React.DOM.i( {className:"icon ion-checkmark"})
          )
        )
      );
    }
  });

  var HostForm = React.createClass({displayName: 'HostForm',
    getInitialState: function () {
      return {titleColor: this.props.titleColor || ""};
    },

    componentDidMount: function () {
      this.refs.attr.getDOMNode().focus();
    },

    onSubmit: function () {
      return this.props.onSubmit(this.refs.attr.getDOMNode().value);
    },

    render : function () {
      return (
        React.DOM.form( {className:"attribute-form pure-form", onSubmit:this.onSubmit}, 
          React.DOM.h2( {className:this.state.titleColor}, "Enter a host"),
          React.DOM.input( {placeholder:"http://example.com", ref:"attr", type:"url",
                 onChange:this.handleChange, defaultValue:this.props.value} ),
          React.DOM.button( {className:"pure-button", type:"submit"}, 
            React.DOM.i( {className:"icon ion-checkmark"})
          )
        )
      );
    }
  });

  if(!localStorage.chat_messages){
    localStorage.chat_messages = JSON.stringify([]);
  }

  React.renderComponent(
    Chat(null ),
    document.getElementById('chat')
  );

}())
